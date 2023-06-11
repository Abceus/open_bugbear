#pragma once
#include <string>
#include <vector>

class BZFArchive {
	struct File {
		std::string path;
		std::vector<unsigned char> buffer;
	};
public:
	void loadArchive(const std::string& path);
	void loadDirectory(const std::string& path);
	
	void saveArchive(const std::string& path) const;
	void saveDirectory(const std::string& path) const;

	void unload();

	void setHeader(const std::string& header);
	void setKey(const std::vector<unsigned char>& key);
private:
	std::vector<unsigned char> header;
	std::vector<unsigned char> key;
	std::vector<File> files;
};