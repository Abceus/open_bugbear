#include "BZFArchive.h"
#include <fstream>
#include <iterator>
#include <string.h>
#include <vcruntime_string.h>
#include <vector>
#include <zlib.h>
#include <cassert>
#include <algorithm>
#include <filesystem>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include < fcntl.h >
#  include < io.h >
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

std::vector<unsigned char> inf(std::vector<unsigned char>& in, int size, int zsize)
{
   int ret;
   unsigned have; 
   z_stream strm;
   
   std::vector<unsigned char> out(size);

   /* Выделение памяти под состояние zlib и инициализация этого состояния */
   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;
   strm.avail_in = 0;
   strm.next_in = Z_NULL;
   ret = inflateInit(&strm);
   if (ret != Z_OK)
      return out;

   /* Декомпрессия до окончания потока deflate или до завершения входного файла */
   do {
	strm.avail_in = zsize;
      strm.next_in = in.data();

      /* Запуск inflate() на входных данных, пока не заполнится выходной буфер */
      do {

         strm.avail_out = size;
         strm.next_out = out.data();


         ret = inflate(&strm, Z_NO_FLUSH);
         assert(ret != Z_STREAM_ERROR);   /* состояние zlib не повреждено */
         switch (ret) {
         case Z_NEED_DICT:
            ret = Z_DATA_ERROR;
         case Z_DATA_ERROR:
         case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return out;
         }

         have = CHUNK - strm.avail_out;

      } while (strm.avail_out == 0);

      /* Завершение, когда inflate() сообщит от этом */
   } while (ret != Z_STREAM_END);

   (void)inflateEnd(&strm);


   return out;
}

std::vector<unsigned char> def(const std::vector<unsigned char>& source, int level)
{
	std::vector<unsigned char> result;
   int ret, flush;
   unsigned have;
   z_stream strm;
   unsigned char in[CHUNK];
   unsigned char out[CHUNK];

   /* Инициализация состояния deflate */
   strm.zalloc = Z_NULL;
   strm.zfree = Z_NULL;
   strm.opaque = Z_NULL;
//    ret = deflateInit(&strm, level);
   ret = deflateInit(&strm, level);
   if (ret != Z_OK)
      return result;

	size_t offset = 0;
   /* Сжатие до конца файла */
   do {

      strm.avail_in = std::min(CHUNK, static_cast<int>(source.size()-offset));
	  memcpy(in, source.data()+offset, strm.avail_in);
	  offset += strm.avail_in;
    //   if (ferror(source)) {
    //      (void)deflateEnd(&strm);
    //      return Z_ERRNO;
    //   }
      flush = strm.avail_in<CHUNK ? Z_FINISH : Z_NO_FLUSH;;
      strm.next_in = in;

      /* Запуск deflate() для обработки входных данных, пока не заполнится
         выходной буфер, с завершением сжатия, когда все данные источника
         будут прочитаны */
      do {

         strm.avail_out = CHUNK;
         strm.next_out = out;

         ret = deflate(&strm, flush);    /* нет плохого значения возврата */
         assert(ret != Z_STREAM_ERROR);  /* этот assert гарантирует, что состояние zlib не испорчено */

         have = CHUNK - strm.avail_out;
		 result.insert(std::end(result), std::begin(out), std::next(std::begin(out), have));
        //  if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        //     (void)deflateEnd(&strm);
        //     return result;
        //  }

      } while (strm.avail_out == 0);
      assert(strm.avail_in == 0);     /* будут использоваться все входные данные */

      /* Завершение, когда были обработаны последние данные файла */
   } while (flush != Z_FINISH);
   assert(ret == Z_STREAM_END);        /* Поток будет завершен */

   /* Очистка и возврат из функции def() */
   (void)deflateEnd(&strm);
   return result;
}


std::vector<unsigned char> encrypt(const std::vector<unsigned char>& key, const std::vector<unsigned char>& input) {
	std::vector<unsigned char> result(input.size());

	for(auto i = 0; i < input.size(); ++i) {
		result[i] = input[i] ^ key[i%key.size()];
	}
	return result;
}

struct FileInfo {
	char type;
	unsigned int offset;
	unsigned int size;
	unsigned int zsize;
	char name[40];
};

void BZFArchive::loadArchive(const std::string& path) {
    std::ifstream input( path, std::ios::binary );

	char start[5] = "    ";
	input.read(start, 4);

	unsigned int date = 0;
	input.read(reinterpret_cast<char*>(&date), sizeof(date));

	unsigned int filesNum = 0;
	input.read(reinterpret_cast<char*>(&filesNum), sizeof(filesNum));


	int size = filesNum * 53;
	std::vector<unsigned char> buffer(size);
	input.read(reinterpret_cast<char*>(buffer.data()), size);
	auto result = encrypt(key, buffer);
	
	input.close();
	for(int i = 0; i < filesNum; ++i) {
		
    	std::ifstream input( path, std::ios::binary );
		int offset = 53*i;

		FileInfo temp;

		memcpy(reinterpret_cast<void*>(&temp.type), reinterpret_cast<void*>(&result[offset]), sizeof(temp.type));
		offset += sizeof(temp.type);

		memcpy(reinterpret_cast<void*>(&temp.offset), reinterpret_cast<void*>(&result[offset]), sizeof(temp.offset));
		offset += sizeof(temp.offset);

		memcpy(reinterpret_cast<void*>(&temp.size), reinterpret_cast<void*>(&result[offset]), sizeof(temp.size));
		offset += sizeof(temp.size);

		memcpy(reinterpret_cast<void*>(&temp.zsize), reinterpret_cast<void*>(&result[offset]), sizeof(temp.zsize));
		offset += sizeof(temp.zsize);

		memcpy(reinterpret_cast<void*>(temp.name), reinterpret_cast<void*>(&result[offset]), sizeof(temp.name));
		offset += sizeof(temp.name);

		auto s = temp.size+header.size();
		input.seekg(temp.offset, std::ios_base::beg);

		std::vector<unsigned char> buffer(s);
		input.read(reinterpret_cast<char*>(buffer.data()), s);
		auto ai = encrypt(key, buffer);

		if(temp.type == 1) {
			ai = inf(ai, s, temp.zsize);
		}

		File result;
		result.path = temp.name;
		result.buffer = std::vector<unsigned char>(std::next(std::begin(ai), header.size()), std::end(ai));
		files.emplace_back(result);
		input.close();
	}
}

void BZFArchive::loadDirectory(const std::string& path) {
	for (const auto& dir_entry : std::filesystem::directory_iterator{path}) {
		if(!dir_entry.is_directory()) {
			File temp;
			temp.path = dir_entry.path().filename().string();
			temp.buffer.resize(dir_entry.file_size());
			
    		std::ifstream input( dir_entry.path(), std::ios::binary );
			input.read(reinterpret_cast<char*>(temp.buffer.data()), temp.buffer.size());
			input.close();

			files.emplace_back(temp);
		}
	}
}

void BZFArchive::saveArchive(const std::string& path) const {
    std::ofstream output( path, std::ios::out | std::ios::binary );

	output.write("bbzf", 4);

	unsigned int date = 0;
	output.write(" ", 4);

	unsigned int filesNum = files.size();
	output.write(reinterpret_cast<char*>(&filesNum), sizeof(filesNum));

	std::vector<FileInfo> filesInfo(filesNum);

	size_t offset = 12 + 53*filesNum;

	std::vector<FileInfo> compressedFiles;
	std::vector<std::vector<unsigned char>> compressedData;

	for(const auto& file: files) {
		auto buffer = file.buffer;
		buffer.insert(std::begin(buffer), std::begin(header), std::end(header));
		auto compressed = encrypt(key, def(buffer, 6));

		FileInfo temp;
		temp.type = 1;
		temp.offset = offset;
		temp.size = file.buffer.size();
		temp.zsize = compressed.size();
		strcpy_s(temp.name, file.path.data());
		if(file.path.size() < sizeof(temp.name)) {
			std::fill(std::next(std::begin(temp.name), file.path.size()), std::end(temp.name), '\0');
		}
		offset += temp.zsize;
		compressedFiles.emplace_back(temp);
		compressedData.emplace_back(compressed);
	}

	std::vector<unsigned char> headerBuffer(compressedFiles.size()*53);
	int headerOffset = 0;
	for(const auto& file: compressedFiles) {
		memcpy(headerBuffer.data()+headerOffset, &file.type, sizeof(file.type));
		headerOffset += sizeof(file.type);

		memcpy(headerBuffer.data()+headerOffset, &file.offset, sizeof(file.offset));
		headerOffset += sizeof(file.offset);

		memcpy(headerBuffer.data()+headerOffset, &file.size, sizeof(file.size));
		headerOffset += sizeof(file.size);

		memcpy(headerBuffer.data()+headerOffset, &file.zsize, sizeof(file.zsize));
		headerOffset += sizeof(file.zsize);

		memcpy(headerBuffer.data()+headerOffset, file.name, sizeof(file.name));
		headerOffset += sizeof(file.name);
	}

	auto cryptedHeader = encrypt(key, headerBuffer);
	output.write(reinterpret_cast<char*>(cryptedHeader.data()), cryptedHeader.size());

	for(const auto& file: compressedData) {
		output.write(reinterpret_cast<const char*>(file.data()), file.size());
	}

	output.close();
}

void BZFArchive::saveDirectory(const std::string& path) const {
	if(!std::filesystem::exists(path)) {
		std::filesystem::create_directory(path);
	}

	std::filesystem::path root = path;

	for(const auto& file: files) {
		auto filePath = root;
		filePath.append(file.path);
		std::ofstream output( filePath, std::ios::out | std::ios::binary );
		output.write(reinterpret_cast<const char*>(file.buffer.data()), file.buffer.size());
		output.close();
	}
}

void BZFArchive::unload() {
	files.clear();
}

void BZFArchive::setHeader(const std::string& header_) {
	header.clear();
	header.insert(std::begin(header), std::begin(header_), std::end(header_));
	header.emplace_back('\0');
}

void BZFArchive::setKey(const std::vector<unsigned char>& key_) {
	key = key_;
}