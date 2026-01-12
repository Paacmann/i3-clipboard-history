#ifndef ClipboardManager_HPP
#define ClipboardManager_HPP

#include <bits/c++config.h>
#include <cstddef>
#include <string>
#include <pthread.h>

#define SHARED_NAME "/clipboardhistory_shm"
#define MAX_ITEMS 20
#define MAX_LENGTH 4096

struct SharedClipboard {
	pthread_mutex_t mutex;
	size_t count;
	char items[MAX_ITEMS][MAX_LENGTH];

};


class ClipboardHistory {
	public:
		explicit ClipboardHistory(std::size_t maxSize = MAX_ITEMS);
		~ClipboardHistory();

		void updateClipboard();
		void showMenu();
	
	private:
		std::size_t _maxSize;
		std::string _lastValue;

		int _shmFd;
		SharedClipboard* _shm;

		void initSharedMemory(bool create);
		void addItem(const std::string& text);

		std::string readClipboard() const;
		void writeClipboard(const std::string& text) const;
};		


#endif
