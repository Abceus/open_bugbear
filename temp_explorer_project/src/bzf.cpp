#include "bzf.h"
#include <cctype>
#include <corecrt_wstdio.h>
#include <fstream>


#include <vcruntime_string.h>
#include <zlib.h>

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
//    ret = inflateInit2(&strm, -15);
   ret = inflateInit(&strm);
   if (ret != Z_OK)
      return out;

   /* Декомпрессия до окончания потока deflate или до завершения входного файла */
   do {
	strm.avail_in = zsize;
    //   strm.avail_in = fread(in, 1, CHUNK, source);
    //   if (ferror(source)) {
    //      (void)inflateEnd(&strm);
    //      return result;
    //   }
    //   if (strm.avail_in == 0)
    //      break;
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
        //  if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
        //     (void)inflateEnd(&strm);
        //     return result;
        //  }

      } while (strm.avail_out == 0);

      /* Завершение, когда inflate() сообщит от этом */
   } while (ret != Z_STREAM_END);

   /* Очистка и возврат */
   (void)inflateEnd(&strm);
//    result.resize(out.size());


   return out;
}


std::vector<unsigned char> encrypt(std::vector<unsigned char> key, std::ifstream& input, int size) {
	std::vector<unsigned char> result(size);

	input.read(reinterpret_cast<char*>(result.data()), size);

	for(auto i = 0; i < size; ++i) {
		result[i] ^= key[i%key.size()];
	}
	return result;
}

  BZFArchive::BZFArchive(const Ogre::String &name, const Ogre::String &archType) : Ogre::Archive(name, archType) {
	auto q = {'\x8b','\xc2','\x69','\x80','\x46','\x42','\x18','\x25','\x80','\x5d','\xaa','\x02','\x6d','\x49','\x11','\x94','\xa8','\x01','\xe7','\x58','\xcb','\x8d','\xa6','\x30','\x3e','\x83','\x1b','\x09','\x4c','\xce','\x18','\xbb','\x7c','\xae','\x6a','\x30','\x1e','\x00','\x0a','\xc9','\x24','\xc0','\x98','\xfd','\x33','\xcc','\x04','\x44','\xfa','\xea','\x8f','\x33','\xc3','\x0d','\xf2','\xe8','\x47','\xd4','\xdc','\xa8','\xc7','\x54','\xa2','\xc7','\x58','\x17','\x32','\xc8','\x7f','\x64','\x4a','\xc6','\xfd','\xc1','\xe7','\x13','\xec','\xb7','\xff','\x1e','\x0b','\xd5','\x72','\x9a','\x57','\xf8','\x3f','\xdc','\xdb','\xc9','\xf3','\x85','\xc8','\x86','\x68','\x62','\xc8','\x06','\xab','\x90','\x90','\xf7','\x3e','\xe4','\xb4','\x6b','\x80','\x89','\xc0','\x93','\x69','\xeb','\x84','\xc9','\x7a','\xd4','\xaf','\xd4','\xf4','\xd5','\xa0','\x68','\x49','\xe8','\x78','\xed','\xd0','\x53','\x74','\x81','\xbc','\xd0','\x7a','\x40','\x4f','\xea','\xf1','\xc3','\x4b','\xaa','\xd6','\xe6','\xaa','\x73','\x0c','\xcf','\x8e','\x2b','\xf4','\x2c','\x7b','\x9b','\x3e','\xbb','\xc4','\x19','\xff','\x10','\x43','\x64','\x03','\x93','\x4d','\xcf','\x64','\xc8','\xe5','\xa2','\x5b','\xd3','\x30','\xbd','\x57','\x3b','\x2a','\x7e','\x4c','\xee','\x96','\xe5','\x4e','\x85','\x3b','\xf6','\x5e','\xca','\x4d','\x5f','\x84','\x77','\x2a','\x5b','\x1e','\x41','\x47','\xd6','\x77','\x15','\x69','\xd2','\x9b','\xa2','\x18','\x09','\x6a','\x17','\x50','\xd4','\xec','\x2e','\x7b','\x4b','\xe4','\x69','\x9c','\xae','\xa8','\x9b','\xcd','\x04','\x2f','\xab','\xeb','\x01','\x6c','\x95','\x90','\x2d','\xda','\xb1','\x41','\x43','\x59','\x38','\xe9','\xd7','\x38','\x04','\x86','\x3c','\x93','\x97','\x23','\xa5','\xdf','\x5f','\x06','\x8a','\xc3','\x39','\x2a','\x4e','\x29','\x33','\xf0','\x1e'};
	
	std::vector<char> key2;
	key.clear();
	key.reserve(q.size());
	for(const auto& q1: q) {
		key.emplace_back(q1);
	}
  }

  BZFArchive::~BZFArchive() {
	unload();
  }

  /// @copydoc Archive::load
  void BZFArchive::load() {
    input.open( getName(), std::ios::binary );

	char start[5] = "    ";
	input.read(start, 4);

	unsigned int date = 0;
	input.read(reinterpret_cast<char*>(&date), sizeof(date));

	unsigned int filesNum = 0;
	input.read(reinterpret_cast<char*>(&filesNum), sizeof(filesNum));


	int size = filesNum * 53;
	auto result = encrypt(key, input, size);
	
	for(int i = 0; i < filesNum; ++i) {
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

		memcpy(reinterpret_cast<void*>(temp.name), reinterpret_cast<void*>(&result[offset]), 40);
		offset += 40;

		std::transform(std::begin(temp.name), std::end(temp.name), std::begin(temp.name),
    		[](unsigned char c){ return std::tolower(c); });

		files.emplace(temp.name, temp);
	}

	input.close();
  }

  /// @copydoc Archive::unload
  void BZFArchive::unload() {
	input.close();
	files.clear();
  }

  /// @copydoc Archive::open
  Ogre::DataStreamPtr BZFArchive::open(const Ogre::String &filename, bool readOnly) const {
	std::string lowercaseFilename;

	for(const auto& c: filename) {
		lowercaseFilename += std::tolower(c);
	}

	auto fileinfo = files.find(lowercaseFilename);
	if(fileinfo != std::end(files)) {
    	input.open( getName(), std::ios::binary );
		auto s = fileinfo->second.size+43;
		long long c1 = input.tellg();
		input.seekg(fileinfo->second.offset, std::ios_base::beg);
		long long c2 = input.tellg();
		auto opened = input.is_open();

		auto ai = encrypt(key, input, s);
		long long c22 = input.tellg();

		auto decompressed = inf(ai, s, fileinfo->second.zsize);

		Ogre::MemoryDataStream stream(decompressed.data()+43, fileinfo->second.size, false);
		Ogre::DataStreamPtr ptr(new Ogre::MemoryDataStream(stream, true));
		long long c3 = input.tellg();
		input.close();
		return ptr;
	}

	return nullptr;
  }

  /// @copydoc Archive::list
  Ogre::StringVectorPtr BZFArchive::list(bool recursive, bool dirs) const {
	Ogre::StringVectorPtr result(new Ogre::StringVector());

	for(const auto& file: files) {
		result->emplace_back(file.first);
	}

	return result;
  }

  /// @copydoc Archive::listFileInfo
  Ogre::FileInfoListPtr BZFArchive::listFileInfo(bool recursive, bool dirs) const {
	Ogre::FileInfoListPtr result(new Ogre::FileInfoList());

	for(const auto& file: files) {
		Ogre::FileInfo temp;
		temp.archive = this;
		temp.filename = file.first;
		temp.path = file.first;
		temp.basename = file.first;
		temp.compressedSize = file.second.zsize;
		temp.uncompressedSize = file.second.size;

		result->emplace_back(temp);
	}

	return result;
  }

  /// @copydoc Archive::find
  Ogre::StringVectorPtr BZFArchive::find(const Ogre::String &pattern, bool recursive, bool dirs) const {
	Ogre::StringVectorPtr result(new Ogre::StringVector());

	std::string lowercasePattern;


	for(const auto& c: pattern) {
		lowercasePattern += std::tolower(c);
	}

	auto wildCard = lowercasePattern.find('*') != std::string::npos;

	if(wildCard) {
		for(const auto& file: files) {
			result->emplace_back(file.first);
		}
	}
	else {
		auto found = files.find(lowercasePattern);
		if(found != std::end(files)) {
			result->emplace_back(found->first);
		}
	}
	return result;
  }

  /// @copydoc Archive::findFileInfo
  Ogre::FileInfoListPtr BZFArchive::findFileInfo(const Ogre::String &pattern, bool recursive, bool dirs) const {
	Ogre::FileInfoListPtr result(new Ogre::FileInfoList());

	std::string lowercasePattern;

	for(const auto& c: pattern) {
		lowercasePattern += std::tolower(c);
	}

	auto wildCard = lowercasePattern.find('*') != std::string::npos;

	if(wildCard) {
		for(const auto& file: files) {
			Ogre::FileInfo temp;
			temp.archive = this;
			temp.filename = file.first;
			temp.path = file.first;
			temp.basename = file.first;
			temp.compressedSize = file.second.zsize;
			temp.uncompressedSize = file.second.size;
			result->emplace_back(temp);
		}
	}
	else {
		auto found = files.find(lowercasePattern);
		if(found != std::end(files)) {
			Ogre::FileInfo temp;
			temp.archive = this;
			temp.filename = found->first;
			temp.path = found->first;
			temp.basename = found->first;
			temp.compressedSize = found->second.zsize;
			temp.uncompressedSize = found->second.size;
			result->emplace_back(temp);
		}
	}
	return result;
  }

  /// @copydoc Archive::exists
  bool BZFArchive::exists(const Ogre::String &filename) const {
	std::string lowercasePattern;

	for(const auto& c: filename) {
		lowercasePattern += std::tolower(c);
	}

	auto found = files.find(lowercasePattern);
	return found != std::end(files);
  }

  const Ogre::String& BZFArchiveFactory::getType(void) const
{
  static Ogre::String name = "BZF";
  return name;
}

  Ogre::Archive* BZFArchiveFactory::createInstance(const Ogre::String &name, bool readOnly) OGRE_NODISCARD {
	return new BZFArchive(name, "BZF");
  }