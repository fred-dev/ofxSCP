//
//  ofxSCP.h
//  openframeworks
//
//  Created by Fred Rodrigues on 10/01/2023.
//

#ifndef ofxSCP_h
#define ofxSCP_h

#pragma once

#include "ofMain.h"
#include <atomic>


class ofxSCP : public ofThread {
public:

        
    // Event for notifications
    ofEvent<string> copyCompleteEvent;
    
    // Constructor
    ofxSCP() : running(false) {
            #ifdef _WIN32
    // check if OpenSSH is installed
    int result = system("where ssh");
    if (result != 0) {
        ofLogError("ofxSCP") << "OpenSSH is not installed, class will not initialize";
        return;
    }
    #endif
    }
    
    ~ofxSCP(){
        stopThread();
        waitForThread(false);
    }
    
    // Thread function
    void threadedFunction(){
        Item item;
        while(isThreadRunning()) {
            // Check if there are any items in the queue
            if (queue.tryReceive(item)) {
                // Set up the command to be executed
                string command = "scp ";
                if (item.recursive) command += "-r ";
                command += item.src + " " + item.user + "@" + item.host + ":" + item.dst;
                
                // Execute the command using `system()`
                string result = ofSystem(command);
                ofLog(OF_LOG_VERBOSE, "ofxSCP::threadedFunction Result of command: " + ofToString(result));
                
                // Notify the main thread that the copy is complete
                ofNotifyEvent(copyCompleteEvent, item.src, this);
            } else {
                // If the queue is empty, stop the thread
                stopThread();
                running = false;
            }
        }
    }
    void send(string host, string user, string src, string dst, bool recursive = false){
        Item item;
        item.host = host;
        item.user = user;
        item.src = add_single_quotes(src);
        item.dst = add_single_quotes(dst);
        item.recursive = recursive;
        queue.send(item);
        
        // If the thread is not running, start it
        if (!running) {
            startThread();
            running = true;
        }
    }
private:
    // Structure for storing items in the queue
    struct Item {
        string host;        // Hostname or IP address of the remote machine
        string user;        // Username for the remote machine
        string src;         // Source file or directory to be copied
        string dst;         // Destination directory on the remote machine
        bool recursive;    // Flag indicating whether to copy directories recursively
    };
    
    ofThreadChannel<Item> queue;    // Thread channel for the copy queue
    std::atomic<bool> running;        // Flag indicating whether the thread is running
    
    string add_single_quotes(string input) {
      
         return "'" + input + "'";;
    }
};


#endif /* ofxSCP.h */
