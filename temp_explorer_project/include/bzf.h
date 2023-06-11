#pragma once

#include <fstream>
#include <OgrePrerequisites.h>

#include <OgreArchive.h>
#include <OgreArchiveFactory.h>

/** Specialisation of the Archive class to allow reading of files from a URL.
 */
class BZFArchive : public Ogre::Archive {
	struct FileInfo {
		char type;
		unsigned int offset;
		unsigned int size;
		unsigned int zsize;
		char name[40];
	};
public:
  BZFArchive(const Ogre::String &name, const Ogre::String &archType);
  ~BZFArchive();

  /// @copydoc Archive::isCaseSensitive
  bool isCaseSensitive(void) const override { return false; }

  /// @copydoc Archive::load
  void load() override;
  /// @copydoc Archive::unload
  void unload() override;

  /// @copydoc Archive::open
  Ogre::DataStreamPtr open(const Ogre::String &filename, bool readOnly=true) const override;

  /// @copydoc Archive::list
  Ogre::StringVectorPtr list(bool recursive = true, bool dirs = false) const override;

  /// @copydoc Archive::listFileInfo
  Ogre::FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false) const override;

  /// @copydoc Archive::find
  Ogre::StringVectorPtr find(const Ogre::String &pattern, bool recursive = true,
							 bool dirs = false) const override;

  /// @copydoc Archive::findFileInfo
  Ogre::FileInfoListPtr findFileInfo(const Ogre::String &pattern,
									 bool recursive = true, bool dirs = false) const override;

  /// @copydoc Archive::exists
  bool exists(const Ogre::String &filename) const override;

	time_t 	getModifiedTime (const Ogre::String &filename) const override { return 0; }
private:
	std::map<std::string, FileInfo> files;

	std::vector<unsigned char> key;

	mutable std::ifstream input;
};

/** Specialisation of ArchiveFactory for URLArchive files. */
class _OgrePrivate BZFArchiveFactory : public Ogre::ArchiveFactory {
public:
  virtual ~BZFArchiveFactory() {}
  /// @copydoc FactoryObj::getType
  const Ogre::String &getType(void) const override;
  /// @copydoc FactoryObj::createInstance
  Ogre::Archive *createInstance(const Ogre::String &name, bool readOnly) OGRE_NODISCARD override;
  /// @copydoc FactoryObj::destroyInstance
  void destroyInstance(Ogre::Archive *arch) override { delete arch; }
};