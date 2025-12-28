#ifndef ClipboardManager_HPP
#define ClipboardManager_HPP

#include <bits/c++config.h>
#include <deque>
#include <string>
#include <mutex>

class ClipboardHistory {
	public:
		explicit ClipboardHistory(std::size_t _maxSize = 20);
		void showMenu();
		void updateClipboard();

	private:
		std::deque<std::string> _history;
		std::size_t _maxSize;
		std::string _lastValue;
		std::mutex _mutex;

		void addItem(const std::string& text);
		std::string readClipboard() const;
		void writeClipboard(const std::string& text) const;

};	


#endif
