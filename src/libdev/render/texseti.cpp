/*
 * T E X S E T I . C P P
 * (c) Charybdis Limited, 1998. All Rights Reserved
 */

//  Definitions of non-inline non-template methods and global functions

#include "render/internal/texseti.hpp"
#include <string>
#include "base/istrrep.hpp"
#include "system/pathname.hpp"
#include "system/fileenum.hpp"
#include "system/filedata.hpp"
#include "render/surfmgr.hpp"

#define TEXSET_STREAM(x)	RENDER_STREAM(x)
#define TEXSET_INDENT(x)	RENDER_INDENT(x)

RenTextureSetImpl::RenTextureSetImpl( void )
: isLoaded_( false )
{
    TEST_INVARIANT;
}

RenTextureSetImpl::RenTextureSetImpl( const SysPathName& directory )
: isLoaded_( false )
{
	load( directory );

    TEST_INVARIANT;
}

RenTextureSetImpl::RenTextureSetImpl( const SysPathName& directory, BaseProgressReporter* pReporter )
: isLoaded_( false )
{
	PRE( pReporter );

	load( directory, pReporter );

    TEST_INVARIANT;
}

RenTextureSetImpl::~RenTextureSetImpl()
{
    TEST_INVARIANT;
	TEXSET_STREAM("Unloading textures " << std::endl);
	TEXSET_INDENT(2);
	TEXSET_STREAM(RenSurfaceManager::instance());

	textures_.erase(textures_.begin(), textures_.end());

	TEXSET_STREAM("Unloaded" << std::endl);
	TEXSET_STREAM(RenSurfaceManager::instance());
	TEXSET_INDENT(-2);
}

// Recognised texture file extensions - kept in sync with texbody.cpp's list.
static const char* const RECOGNISED_TEXTURE_EXTENSIONS[] = { "bmp", "png", "jpg", "tga" };

// Returns true if pathname's filename ends with "_<literal>.<ext>" for any
// recognised extension (case-insensitive).
static bool matchesLiteralSuffix(const SysPathName& pathname, const std::string& literal)
{
	const std::string& texName = pathname.filename();
	for (size_t i = 0; i < sizeof(RECOGNISED_TEXTURE_EXTENSIONS) / sizeof(RECOGNISED_TEXTURE_EXTENSIONS[0]); ++i)
	{
		const std::string candidate = "_" + literal + "." + RECOGNISED_TEXTURE_EXTENSIONS[i];
		if (texName.length() >= candidate.length() &&
			strcasecmp(texName.substr(texName.length() - candidate.length()).c_str(), candidate.c_str()) == 0)
			return true;
	}
	return false;
}

static bool isAlphaMap( const SysPathName& pathname )
{
	return matchesLiteralSuffix(pathname, "a") || matchesLiteralSuffix(pathname, "ba");
}

bool isColourMap( const SysPathName& pathname )
{
	return matchesLiteralSuffix(pathname, "c") || matchesLiteralSuffix(pathname, "bc");
}

void RenTextureSetImpl::load( const SysPathName& directory, BaseProgressReporter* pReporter )
{
	PRE( not isLoaded_ );

	TEXSET_STREAM("Preloading textures from directory " << directory << std::endl);
	TEXSET_INDENT(2);
	TEXSET_STREAM(RenSurfaceManager::instance());

	// SysFileEnumerator only takes one file spec per instance, so scan once per
	// recognised extension and combine the results.
	ctl_vector< SysFileData > files;
	for (size_t i = 0; i < sizeof(RECOGNISED_TEXTURE_EXTENSIONS) / sizeof(RECOGNISED_TEXTURE_EXTENSIONS[0]); ++i)
	{
		const std::string spec = std::string("*.") + RECOGNISED_TEXTURE_EXTENSIONS[i];
		SysFileEnumerator fileFinder( directory, spec.c_str() );
		fileFinder.examineSubdirectories( true );
		fileFinder.find();
		const ctl_vector< SysFileData >& found = fileFinder.files();
		for (ctl_vector< SysFileData >::const_iterator it = found.begin(); it != found.end(); ++it)
			files.push_back( *it );
	}

	TEXSET_STREAM("Found " << files.size() << " texture files" << std::endl);
	textures_.reserve( files.size() );

	size_t filesRead = 0;
	size_t numFiles = files.size();
	size_t reportWhenFilesRead = 1;

	for( ctl_vector< SysFileData >::const_iterator it = files.begin(); it!=files.end(); ++it )
	{
		if ( pReporter )
		{
			++filesRead;
			if ( filesRead == reportWhenFilesRead )
			{
				size_t inc = pReporter->report( filesRead,numFiles );
				if ( inc == 0 )
					inc = 1;
				reportWhenFilesRead += inc;
			}
		}

		const SysPathName& fileName = (*it).pathName();
		// Check if the file has a _ba.bmp or _bc.bmp termination,
		// in which only load if
		// we are considering a color or a transparancy bitmap, do not load
		// RenSurfBody::read will sort out which bitmap effectively needs to be loaded
		if( not isAlphaMap( fileName ) and not isColourMap( fileName ) )
		{

			TEXSET_STREAM(" (" << textures_.size() << ") preloading texture "<< fileName.pathname() << std::endl);
			// load a texture and save the texture handle in textures_
			// Note: This call does not make use of the directory search
			// mechanism since (*i) refers to an absolute pathname (see
			// RenISurfaceManagerImpl::createSurfOrTex)
			RenTexture texture = RenSurfaceManager::instance(). createTexture( fileName.pathname() );
			textures_.push_back( texture );
		}
	}

	TEXSET_STREAM("Loaded " << textures_.size() << " bitmaps files" << std::endl);
	isLoaded_=true;

	TEXSET_STREAM(RenSurfaceManager::instance());
	TEXSET_INDENT(-2);
	TEXSET_STREAM("Done preloading textures from directory " << directory << std::endl);
}

void RenTextureSetImpl::CLASS_INVARIANT
{
    INVARIANT( this != NULL );
}


ostream& operator <<( ostream& o, const RenTextureSetImpl& t )
{

    o << "RenTextureSetImpl " << (void*)&t << " start" << std::endl;
    o << "RenTextureSetImpl " << (void*)&t << " end" << std::endl;

    return o;
}

/* End TEXSETI.CPP **************************************************/
