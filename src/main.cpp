#include "../include/ClipboardManager.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>

int main(int argc, char** argv) {

	ClipboardHistory clipboard(20);

    // Ako je pozvan sa --menu
    	if (argc > 1 && std::strcmp(argv[1], "--menu") == 0) {
        	clipboard.showMenu();
        	return 0;
    	}

    // Inače radi kao daemon
    	while (true) {
        	clipboard.updateClipboard();
        	std::this_thread::sleep_for(std::chrono::milliseconds(500));
   	}
}

