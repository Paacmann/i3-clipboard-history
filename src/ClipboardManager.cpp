#include "../include/ClipboardManager.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ostream>
#include <pthread.h>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>


ClipboardHistory::ClipboardHistory(size_t maxSize)
	: _maxSize(maxSize) , _shmFd(-1) , _shm(nullptr)
	{
		bool create = false;
		_shmFd = shm_open(SHARED_NAME, O_RDWR, 0666);

		if (_shmFd == -1) {
    			create = true;
		}
		
		initSharedMemory(create);
	}




ClipboardHistory::~ClipboardHistory() {
	if (_shm) {
		munmap(_shm, sizeof(SharedClipboard));
	}

	if (_shmFd != -1)
		close(_shmFd);

}


void ClipboardHistory::initSharedMemory(bool create) {
	int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;

	_shmFd = shm_open(SHARED_NAME,flags,0666);
	if (_shmFd == -1) {
		perror("shm open error!");
		std::exit(1);
	}

	if (create) {
		ftruncate(_shmFd , sizeof(SharedClipboard));
	}


	_shm = static_cast<SharedClipboard*>(
			mmap(nullptr,sizeof(SharedClipboard),PROT_READ | PROT_WRITE,MAP_SHARED,_shmFd,0));

	
	if (_shm == MAP_FAILED) {
		perror("mmap failed!");
		std::exit(1);
	}

	if (create) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setpshared(&attr,PTHREAD_PROCESS_SHARED);

		pthread_mutex_init(&_shm->mutex, &attr);
		pthread_mutexattr_destroy(&attr);
		_shm->count = 0;
	
	}
}




std::string ClipboardHistory::readClipboard() const {
	FILE* pipe = popen("xclip -o -selection clipboard 2>/dev/null", "r");
	if (!pipe) {
		return "";
	}

	char buffer[MAX_LENGTH];
	std::string res;
	while (fgets(buffer,sizeof(buffer),pipe)) {
		res += buffer;
	}
	
	pclose(pipe);
	return res;
}

void ClipboardHistory::writeClipboard(const std::string& text) const {
	FILE* pipe = popen("xclip -i -selection clipboard", "w");
	if (!pipe)
		return;

	fwrite(text.c_str(),1,text.size(),pipe);
	pclose(pipe);
}


void ClipboardHistory::addItem(const std::string& text) {
	pthread_mutex_lock(&_shm->mutex);

	if (_shm->count > 0 && std::strcmp(_shm->items[0],text.c_str()) == 0) {
		pthread_mutex_unlock(&_shm->mutex);
		return;
	}


	if (_maxSize == 0) {
    		pthread_mutex_unlock(&_shm->mutex);
    		return;
	}


	if (_shm->count == _maxSize) {
		for (size_t i = _shm->count - 1; i > 0; i--) {
			std::strcpy(_shm->items[i],_shm->items[i-1]);
		}

	} else {
		for (size_t i = _shm->count; i > 0; i--) {
			std::strcpy(_shm->items[i],_shm->items[i-1]);
		}

		_shm->count++;
	}	
	
	std::strncpy(_shm->items[0], text.c_str(), MAX_LENGTH - 1);
	_shm->items[0][MAX_LENGTH - 1] = '\0';

	pthread_mutex_unlock(&_shm->mutex);
}

void ClipboardHistory::updateClipboard() {
	std::string current = readClipboard();
	if (current.empty() || current == _lastValue)
		return;

	_lastValue = current;
	addItem(current);
}

void ClipboardHistory::showMenu() {
	pthread_mutex_lock(&_shm->mutex);

	if (_shm->count == 0) {
		pthread_mutex_unlock(&_shm->mutex);
		std::cerr << "Clipboard is empty";
		return;
	}

	std::ostringstream menu;
	for (size_t i = 0; i < _shm->count; i++)
		menu << _shm->items[i] << "\n";

	pthread_mutex_unlock(&_shm->mutex);


	
	std::string cmd = "echo \"" + menu.str() + "\" | dmenu -i -l 10";
	
	FILE* pipe = popen(cmd.c_str(), "r");
    	
	if (!pipe)
		return;

    	char buffer[MAX_LENGTH];
    	std::string selected;

    	if (fgets(buffer, sizeof(buffer), pipe)) {
        	selected = buffer;
        	if (!selected.empty() && selected.back() == '\n')
            		selected.pop_back();
    	}

    	pclose(pipe);

    	if (!selected.empty()) {
        	writeClipboard(selected);
    	}
	
} 










