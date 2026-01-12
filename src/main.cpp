#include "../include/ClipboardManager.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

int main(int argc, char** argv) {

	ClipboardHistory clipboard(20);

    	// --menu process
    	if (argc > 1 && std::strcmp(argv[1], "--menu") == 0) {
        	clipboard.showMenu();
        	return 0;
    	}

	// deamon process
    	while (true) {
        	clipboard.updateClipboard();
        	std::this_thread::sleep_for(std::chrono::milliseconds(500));
   	}
}

