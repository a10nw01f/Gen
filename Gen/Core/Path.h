#pragma once
#include "std.h"
#include "FixedString.h"
#include "Utils.h"

namespace Gen
{
    constexpr FixedString GetCurrentDir(const char* path)
    {
        std::string str(path);
        return StringToFixedStr(str.substr(0, str.find_last_of('\\')).c_str());
    }

    struct NameAndFolder
    {
        FixedString m_Name;
        FixedString m_FolderPath;
        constexpr NameAndFolder(const char* name, std::source_location source_location = std::source_location::current()) :
            m_Name(StringToFixedStr(name)),
            m_FolderPath(StringToFixedStr(GetCurrentDir(source_location.file_name()).data()))
        {
        }

        constexpr auto PathWithExt(const char* ext) const
        {
            return m_FolderPath.String() + std::string("\\") + m_Name.data() + ext;
        }
    };

    template<NameAndFolder file_info, StaticString ext>
    constexpr auto PathWithExt() {
        return StringToArray_v<[]{
            return file_info.PathWithExt(ext.m_Array);
        }>;
    }

    struct Path {
        FixedString m_RelativePath;
        FixedString m_Dir;

        constexpr Path(const char* relative_path, std::source_location source_location = std::source_location::current()) :
            m_RelativePath(StringToFixedStr(relative_path)), 
            m_Dir(StringToFixedStr(GetCurrentDir(source_location.file_name()).data()))
        {}
    };

    template<Path path>
    constexpr auto FullPath() {
        return StringToArray_v<[]{
            return path.m_Dir.String() + std::string("\\") + path.m_RelativePath.data();
        }>;
    }
}