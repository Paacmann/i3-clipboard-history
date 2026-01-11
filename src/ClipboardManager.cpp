#include "../include/ClipboardManager.hpp"
#include <cstddef>
#include <fstream>
#include <ios>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <mutex>
#include <ostream>
#include <sstream>
#include <string>


ClipboardHistory::ClipboardHistory(size_t maxSize)
	: _maxSize(maxSize)
	{}


std::string ClipboardHistory::readClipboard() const {

	FILE* pipe = popen("xclip -o -selection clipboard 2>/dev/null","r");
	if (!pipe)
		return "";
	
	char buffer[4096];
	std::string result;

	while (fgets(buffer , sizeof(buffer) , pipe)) {
		result += buffer;
	}
	
	pclose(pipe);

	return result;

}


void ClipboardHistory::writeClipboard(const std::string& text) const {
	FILE* pipe = popen("xclip -i -selection clipboard", "w");
	if (!pipe)
		return;
	
	size_t written = fwrite(text.c_str(),1,text.size(), pipe);
	if (written != text.size())
		std::cerr << "Warning: fwrite failed!";
	
	pclose(pipe);
}


void ClipboardHistory::addItem(const std::string& text) {
	
	std::lock_guard<std::mutex> lock(_mutex);
	//auto it = std::find(_history.begin(),_history.end(),text);
	//if (it != _history.end())
	//	_history.erase(it);
	auto history = loadHistory();
	if (!history.empty() && history.front() == text)
		return;

	
	history.push_front(text);
	if (history.size() > _maxSize)
		history.pop_back();
	
	saveHistory(history);
}


void ClipboardHistory::updateClipboard() {
	
	std::string current = readClipboard();

	if (current.empty() || current == _lastValue)
		return ;
	
	_lastValue = current;
	addItem(current);
}

void ClipboardHistory::showMenu() {
	// zakljucavam , stitim _history
	//std::lock_guard<std::mutex> lock(_mutex);

	auto history = loadHistory();	
	if (history.empty()) {
		std::cerr << "ClipboardHistory is empty \n";
		return;
	}

	// pripremam string sa stavkama za menu
	std::ostringstream menuStream;
	for (const auto& element : history)
		menuStream << element << "\n";

	std::string menu = menuStream.str(); // ovo proveri 
	
	
	std::string cmd = "echo \"" + menu + "\" | dmenu -i -l 10";

	// pokrecemo dmenu , saljemo menu na stdin
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		std::cerr << "Failed to open dmenu\n";
		return;
	}
	
	//fwrite(menu.c_str(),1,menu.size(),pipe);
	//fflush(pipe); // flush da dmenu vidi sadrzaj
	
	// citamo odabranu stavku
	char buffer[4096];
	std::string selected;
	if (fgets(buffer,sizeof(buffer),pipe)) {
		selected = buffer;
		if (!selected.empty() && selected.back() == '\n')
			selected.pop_back();
	}

	pclose(pipe);

	if (selected.empty()) return;

	writeClipboard(selected);

	std::cout << "Copied to Clipboard: " << selected << "\n";
}

static const char* SHARED_FILE = "/tmp/clipboardhistory.txt";

void ClipboardHistory::saveHistory(const std::deque<std::string>& history) const {
	std::ofstream out(SHARED_FILE, std::ios::trunc);
	if (!out.is_open())
		return ;
	for (const auto &elem : history)
		out << elem << '\n';

}

std::deque<std::string> ClipboardHistory::loadHistory() const {
	std::deque<std::string> history;
	std::ifstream in(SHARED_FILE);

	if (!in.is_open())
		return history;

	std::string line;
	while (std::getline(in,line)) {
		if (!line.empty())
			history.push_back(line);
	}
	
	return history;
}
