/*  Clipboard - Cut, copy, and paste anything, anywhere, all from the terminal.
    Copyright (C) 2023 Jackson Huff and other contributors on GitHub.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
#pragma once
#include <vector>
#include <filesystem>
#include <string_view>
#include <array>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <clipboard/gui.hpp>

namespace fs = std::filesystem;

struct Filepath {
    fs::path main;
    fs::path temporary;
    fs::path persistent;
    fs::path original_files;
    fs::path home;
};
extern Filepath filepath;

struct Copying {
    bool is_persistent = false;
    bool use_safe_copy = true;
    fs::copy_options opts = fs::copy_options::overwrite_existing | fs::copy_options::recursive | fs::copy_options::copy_symlinks;
    std::vector<fs::path> items;
    std::vector<std::pair<std::string, std::error_code>> failedItems;
    std::string buffer;
};
extern Copying copying;

extern std::string clipboard_name;

enum class SpinnerState : int { Done, Active, Cancel };

extern std::condition_variable cv;
extern std::mutex m;
extern std::atomic<SpinnerState> spinner_state;
extern std::thread indicator;

struct Successes {
    std::atomic<unsigned long> files;
    std::atomic<unsigned long> directories;
    std::atomic<unsigned long long> bytes;
};
extern Successes successes;

struct IsTTY {
    bool std_in = true;
    bool std_out = true;
    bool std_err = true;
};
extern IsTTY is_tty;

struct Constants {
    std::string_view clipboard_version = "0.2.1r2";
    std::string_view pipe_file = "clipboard.rawdata";
    std::string_view default_clipboard_name = "0";
    std::string_view temporary_directory_name = "Clipboard";
    std::string_view persistent_directory_name = ".clipboard";
    std::string_view original_files_extension = ".files";
};
constexpr Constants constants;

enum class Action : unsigned int { Cut, Copy, Paste, PipeIn, PipeOut, Clear, Show, Edit };
extern Action action;

template <typename T, size_t N>
class ActionArray : public std::array<T, N> {
public:
    T& operator[](Action index) {
        return std::array<T, N>::operator[](static_cast<unsigned int>(index)); //switch to std::to_underlying when available
    }
};

extern ActionArray<std::string_view, 8> actions;
extern ActionArray<std::string_view, 8> action_shortcuts;
extern ActionArray<std::string_view, 8> doing_action;
extern ActionArray<std::string_view, 8> did_action;

static std::array<std::pair<std::string_view, std::string_view>, 8> colors = {{
    {"{red}", "\033[38;5;196m"},
    {"{green}", "\033[38;5;40m"},
    {"{yellow}", "\033[38;5;214m"},
    {"{blue}", "\033[38;5;51m"},
    {"{orange}", "\033[38;5;208m"},
    {"{pink}", "\033[38;5;219m"},
    {"{bold}", "\033[1m"},
    {"{blank}", "\033[0m"}
}};

static std::string replaceColors(const std::string_view& str) {
    std::string temp(str); //a string to do scratch work on
    for (const auto& key : colors) { //iterate over all the possible colors to replace
        for (int i = 0; (i = temp.find(key.first, i)) != std::string::npos; i += key.second.length()) {
            temp.replace(i, key.first.length(), key.second);
        }
    }
    return temp;
}

class Message {
private:
    std::string_view internal_message;
public:
    Message(const auto& message) : internal_message(std::move(message)) {}
    std::string operator()() const { return std::move(replaceColors(internal_message)); }
};

extern Message help_message;
extern Message check_clipboard_status_message;
extern Message clipboard_item_contents_message;
extern Message clipboard_text_contents_message;
extern Message no_clipboard_contents_message;
extern Message clipboard_action_prompt;
extern Message no_valid_action_message;
extern Message choose_action_items_message;
extern Message fix_redirection_action_message;
extern Message redirection_no_items_message;
extern Message paste_success_message;
extern Message clear_success_message;
extern Message clear_fail_message;
extern Message clipboard_failed_message;
extern Message and_more_fails_message;
extern Message and_more_items_message;
extern Message fix_problem_message;
extern Message not_enough_storage_message;
extern Message item_already_exists_message;
extern Message bad_response_message;
extern Message working_message;
extern Message cancelled_message;
extern Message pipe_success_message;
extern Message one_item_success_message;
extern Message multiple_files_success_message;
extern Message multiple_directories_success_message;
extern Message multiple_files_directories_success_message;
extern Message internal_error_message;

void setLanguagePT();
void setLanguageTR();
void setLanguageES();
std::string replaceColors(const std::string_view& str);
void setupHandlers();
void setLocale();
void showHelpMessage(int& argc, char *argv[]);
void setupItems(int& argc, char *argv[]);
void setClipboardName(int& argc, char *argv[]);
void setupVariables(int& argc, char *argv[]);
void createTempDirectory();
void syncWithGUIClipboard(std::string const& text);
void syncWithGUIClipboard(ClipboardPaths const& clipboard);
void syncWithGUIClipboard();
void showClipboardStatus();
void showClipboardContents();
void setupAction(int& argc, char *argv[]);
void checkForNoItems();
bool stopIndicator(bool change_condition_variable);
void startIndicator();
void setupIndicator();
void deduplicateItems();
unsigned long long calculateTotalItemSize();
void checkItemSize();
void clearTempDirectory(bool force_clear);
void copyFiles();
void removeOldFiles();
bool userIsARobot();
int getUserDecision(const std::string& item);
void pasteFiles();
void pipeIn();
void pipeOut();
void clearClipboard();
void performAction();
void updateGUIClipboard();
void showFailures();
void showSuccesses();

extern ClipboardContent getGUIClipboard();
extern void writeToGUIClipboard(ClipboardContent const& clipboard);