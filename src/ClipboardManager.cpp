#include "../include/ClipboardManager.hpp"
#include <cstddef>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <mutex>


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
	auto it = std::find(_history.begin(),_history.end(),text);
	if (it != _history.end())
		_history.erase(it);
	
	_history.push_front(text);
	if (_history.size() > _maxSize)
		_history.pop_back();

}


void ClipboardHistory::updateClipboard() {
	
	std::string current = readClipboard();

	if (current.empty() || current == _lastValue)
		return ;
	
	_lastValue = current;
	addItem(current);
}
