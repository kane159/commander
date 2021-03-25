#include <iostream>
#include <sstream>
#include "panel.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "fileutils.h"

namespace {
#define PANEL_SIZE (screen.w / 2 - 2)
#define NAME_SIZE (PANEL_SIZE - 18)
#define CONTENTS_H (screen.h - HEADER_H - FOOTER_H)
} // namespace

CPanel::CPanel(const std::string &p_path, const Sint16 p_x):
    m_currentPath(""),
    m_camera(0),
    m_x(p_x),
    m_highlightedLine(0),
    m_iconDir(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FOLDER)),
    m_iconFile(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FILE)),
    m_iconImg(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FILE_IMAGE)),
    m_iconIpk(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FILE_INSTALLABLE_PACKAGE)),
    m_iconOpk(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FILE_PACKAGE)),
    m_iconIsSymlink(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_FILE_IS_SYMLINK)),
    m_iconUp(CResourceManager::instance().getSurface(CResourceManager::T_SURFACE_UP)),
    m_fonts(CResourceManager::instance().getFonts())
{
    // List the given path
    if (m_fileLister.list(p_path))
    {
        // Path OK
        m_currentPath = p_path;
    }
    else
    {
        // The path is wrong => take default
        m_fileLister.list(PATH_DEFAULT);
        m_currentPath = PATH_DEFAULT;
    }
}

CPanel::~CPanel(void) { }

SDL_Surface *CPanel::cursor1() const
{
    return CResourceManager::instance().getSurface(
        CResourceManager::T_SURFACE_CURSOR1);
}

SDL_Surface *CPanel::cursor2() const
{
    return CResourceManager::instance().getSurface(
        CResourceManager::T_SURFACE_CURSOR2);
}

void CPanel::render(const bool p_active) const
{
    // Draw panel
    const Sint16 l_x = m_x + m_iconDir->w / screen.ppu_x + 2;
    const unsigned int l_nbTotal = m_fileLister.getNbTotal();
    Sint16 l_y = Y_LIST;
    SDL_Surface *l_surfaceTmp = NULL;
    const SDL_Color *l_color = NULL;
    SDL_Rect l_rect;
    // Current dir
    l_surfaceTmp = SDL_utils::renderText(m_fonts, m_currentPath, Globals::g_colorTextTitle, {COLOR_TITLE_BG});
    if (l_surfaceTmp->w > PANEL_SIZE * screen.ppu_x)
    {
        l_rect.x = l_surfaceTmp->w - PANEL_SIZE * screen.ppu_x;
        l_rect.y = 0;
        l_rect.w = PANEL_SIZE * screen.ppu_x;
        l_rect.h = l_surfaceTmp->h;
        SDL_utils::applySurface(m_x, HEADER_PADDING_TOP, l_surfaceTmp, screen.surface, &l_rect);
    }
    else
    {
        SDL_utils::applySurface(m_x, HEADER_PADDING_TOP, l_surfaceTmp, screen.surface);
    }

    SDL_FreeSurface(l_surfaceTmp);
    SDL_Rect clip_contents_rect = SDL_utils::Rect(0, Y_LIST * screen.ppu_y, screen.w * screen.ppu_x, CONTENTS_H * screen.ppu_y);
    // Content
    SDL_SetClipRect(screen.surface, &clip_contents_rect);
    // Draw cursor
    SDL_utils::applySurface(m_x - 1, Y_LIST + (m_highlightedLine - m_camera) * LINE_HEIGHT, p_active ? cursor1() : cursor2(), screen.surface);
    for (unsigned int l_i = m_camera; l_i < m_camera + NB_VISIBLE_LINES && l_i < l_nbTotal; ++l_i)
    {
        // Icon and color
        if (m_fileLister.isDirectory(l_i))
        {
            // Icon
            if (m_fileLister[l_i].m_name == "..")
                l_surfaceTmp = m_iconUp;
            else
                l_surfaceTmp = m_iconDir;
            // Color
            if (m_selectList.find(l_i) != m_selectList.end())
                l_color = &Globals::g_colorTextSelected;
            else
                l_color = &Globals::g_colorTextDir;
        }
        else
        {
            // Icon
            const std::string &ext = m_fileLister[l_i].m_ext;
            if (SDL_utils::isSupportedImageExt(ext))
                l_surfaceTmp = m_iconImg;
            else if (ext == "ipk")
                l_surfaceTmp = m_iconIpk;
            else if (ext == "opk")
                l_surfaceTmp = m_iconOpk;
            else
                l_surfaceTmp = m_iconFile;
            // Color
            if (m_selectList.find(l_i) != m_selectList.end())
                l_color = &Globals::g_colorTextSelected;
            else
                l_color = &Globals::g_colorTextNormal;
        }
        SDL_utils::applySurface(m_x, l_y, l_surfaceTmp, screen.surface);
        if (m_fileLister[l_i].is_symlink)
            SDL_utils::applySurface(m_x, l_y, m_iconIsSymlink, screen.surface);
        // Text
        SDL_Color l_bg;
        if (l_i == m_highlightedLine) {
            if (p_active)
                l_bg = {COLOR_CURSOR_1};
            else
                l_bg = {COLOR_CURSOR_2};
        } else {
            static const SDL_Color kLineBg[2] = {{COLOR_BG_1}, {COLOR_BG_2}};
            l_bg = kLineBg[(l_i - m_camera) % 2];
        }
        l_surfaceTmp = SDL_utils::renderText(m_fonts, m_fileLister[l_i].m_name, *l_color, l_bg);
        if (l_surfaceTmp->w > NAME_SIZE * screen.ppu_x)
        {
            l_rect.x = 0;
            l_rect.y = 0;
            l_rect.w = NAME_SIZE * screen.ppu_x;
            l_rect.h = l_surfaceTmp->h;
            SDL_utils::applySurface(l_x, l_y + 2, l_surfaceTmp, screen.surface, &l_rect);
        }
        else
        {
            SDL_utils::applySurface(l_x, l_y + 2, l_surfaceTmp, screen.surface);
        }
        SDL_FreeSurface(l_surfaceTmp);
        // Next line
        l_y += LINE_HEIGHT;
    }
    SDL_SetClipRect(screen.surface, nullptr);

    // Footer
    std::string l_footer("-");
    if (!m_fileLister.isDirectory(m_highlightedLine))
    {
        std::ostringstream l_s;
        l_s << m_fileLister[m_highlightedLine].m_size;
        l_footer = l_s.str();
        File_utils::formatSize(l_footer);
    }
    SDL_utils::applyText(m_x + 2, FOOTER_Y + FOOTER_PADDING_TOP, screen.surface, m_fonts, "Size:", Globals::g_colorTextTitle, {COLOR_TITLE_BG});
    SDL_utils::applyText(m_x + PANEL_SIZE - 2, FOOTER_Y + FOOTER_PADDING_TOP, screen.surface, m_fonts, l_footer, Globals::g_colorTextTitle, {COLOR_TITLE_BG}, SDL_utils::T_TEXT_ALIGN_RIGHT);
}

const bool CPanel::moveCursorUp(unsigned char p_step)
{
    if (m_highlightedLine)
    {
        // Move cursor
        if (m_highlightedLine > p_step)
            m_highlightedLine -= p_step;
        else
            m_highlightedLine = 0;
        // Adjust camera
        adjustCamera();
        // Return true for new render
        return true;
    }
    return false;
}

const bool CPanel::moveCursorDown(unsigned char p_step)
{
    const unsigned int l_nb = m_fileLister.getNbTotal();
    if (m_highlightedLine < l_nb - 1)
    {
        // Move cursor
        if (m_highlightedLine + p_step > l_nb - 1)
            m_highlightedLine = l_nb - 1;
        else
            m_highlightedLine += p_step;
        // Adjust camera
        adjustCamera();
        // Return true for new render
        return true;
    }
    return false;
}

int CPanel::getNumVisibleListItems() const
{
    return std::min(
        static_cast<decltype(m_fileLister.getNbTotal())>(NB_VISIBLE_LINES),
        m_fileLister.getNbTotal());
}

int CPanel::getLineAt(int x, int y) const
{
    if (x <= 0 || y <= Y_LIST * screen.ppu_y
        || y > (Y_LIST + getNumVisibleListItems() * LINE_HEIGHT) * screen.ppu_y)
        return -1;
    return (y - Y_LIST * screen.ppu_y) / (LINE_HEIGHT * screen.ppu_y);
}

void CPanel::moveCursorToVisibleLineIndex(int index)
{
    m_highlightedLine = m_camera + index;
}

const bool CPanel::open(const std::string &p_path)
{
    bool l_ret(false);
    std::string l_newPath("");
    std::string l_oldDir("");
    if (p_path.empty())
    {
        // Open highlighted dir
        if (m_fileLister[m_highlightedLine].m_name == "..")
        {
            // Go to parent dir
            size_t l_pos = m_currentPath.rfind('/');
            // Remove the last dir in the path
            l_newPath = m_currentPath.substr(0, l_pos);
            if (l_newPath.empty())
                // We're at /
                l_newPath = "/";
            l_oldDir = m_currentPath.substr(l_pos + 1);
        }
        else
        {
            l_newPath = m_currentPath + (m_currentPath == "/" ? "" : "/") + m_fileLister[m_highlightedLine].m_name;
        }
    }
    else
    {
        // Open given dir
        if (p_path == m_currentPath)
            return false;
        l_newPath = p_path;
    }
    // List the new path
    if (m_fileLister.list(l_newPath))
    {
        // Path OK
        m_currentPath = l_newPath;
        // If it's a back movement, restore old dir
        if (!l_oldDir.empty())
            m_highlightedLine = m_fileLister.searchDir(l_oldDir);
        else
            m_highlightedLine = 0;
        // Camera
        adjustCamera();
        // Clear select list
        m_selectList.clear();
        // New render
        l_ret = true;
    }
    INHIBIT(std::cout << "open - new current path: " << m_currentPath << std::endl;)
    return l_ret;
}

const bool CPanel::goToParentDir(void)
{
    bool l_ret(false);
    // Select ".." and open it
    if (m_currentPath != "/")
    {
        m_highlightedLine = 0;
        l_ret = open();
    }
    return l_ret;
}

void CPanel::adjustCamera(void)
{
    if (m_fileLister.getNbTotal() <= NB_VISIBLE_LINES)
        m_camera = 0;
    else if (m_highlightedLine < m_camera)
        m_camera = m_highlightedLine;
    else if (m_highlightedLine > m_camera + NB_FULLY_VISIBLE_LINES - 1)
        m_camera = m_highlightedLine - NB_FULLY_VISIBLE_LINES + 1;
}

const std::string &CPanel::getHighlightedItem(void) const
{
    return m_fileLister[m_highlightedLine].m_name;
}

const std::string CPanel::getHighlightedItemFull(void) const
{
    return m_currentPath + (m_currentPath == "/" ? "" : "/") + m_fileLister[m_highlightedLine].m_name;
}

const std::string &CPanel::getCurrentPath(void) const
{
    return m_currentPath;
}

const unsigned int &CPanel::getHighlightedIndex(void) const
{
    return m_highlightedLine;
}

const unsigned int CPanel::getHighlightedIndexRelative(void) const
{
    return m_highlightedLine - m_camera;
}

void CPanel::refresh(void)
{
    // List current path
    if (m_fileLister.list(m_currentPath))
    {
        // Adjust selected line
        if (m_highlightedLine > m_fileLister.getNbTotal() - 1)
            m_highlightedLine = m_fileLister.getNbTotal() - 1;
    }
    else
    {
        // Current path doesn't exist anymore => default
        m_fileLister.list(PATH_DEFAULT);
        m_currentPath = PATH_DEFAULT;
        m_highlightedLine = 0;
    }
    // Camera
    adjustCamera();
    // Clear select list
    m_selectList.clear();
}

const bool CPanel::addToSelectList(const bool p_step)
{
    if (m_fileLister[m_highlightedLine].m_name != "..")
    {
        // Search highlighted element in select list
        std::set<unsigned int>::iterator l_it = m_selectList.find(m_highlightedLine);
        if (l_it == m_selectList.end())
            // Element not present => we add it
            m_selectList.insert(m_highlightedLine);
        else
            // Element present => we remove it from the list
            m_selectList.erase(m_highlightedLine);
        if (p_step)
            moveCursorDown(1);
        return true;
    }
    else
    {
        return false;
    }
}

const std::set<unsigned int> &CPanel::getSelectList(void) const
{
    return m_selectList;
}

void CPanel::getSelectList(std::vector<std::string> &p_list) const
{
    p_list.clear();
    // Insert full path of selected files
    for (std::set<unsigned int>::const_iterator l_it = m_selectList.begin(); l_it != m_selectList.end(); ++l_it)
    {
        if (m_currentPath == "/")
            p_list.push_back(m_currentPath + m_fileLister[*l_it].m_name);
        else
            p_list.push_back(m_currentPath + "/" + m_fileLister[*l_it].m_name);
    }
}

void CPanel::selectAll(void)
{
    const unsigned int l_nb = m_fileLister.getNbTotal();
    for (unsigned int l_i = 1; l_i < l_nb; ++l_i)
        m_selectList.insert(l_i);
}

void CPanel::selectNone(void)
{
    m_selectList.clear();
}

const bool CPanel::isDirectoryHighlighted(void) const
{
    return m_fileLister.isDirectory(m_highlightedLine);
}
