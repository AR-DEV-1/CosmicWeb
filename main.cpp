#define COSMIC_WEB_ENGINE
#define COSMIC_WEB_ENGINE_IMPLEMENTATION
#include "include/engine.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/stb_truetype.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>

// Copyright AR-DEV-1
// Copyright The Cosmic Web Authors
// This software and its source is provided under GNU GPL 3.0
// The runtime, software, source and etc are all provided under GNU GPL 3.0

// Cosmic Web Engine Implementation

// Write callback function to accumulate the response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    char** responsePtr = (char**)userp;

    // Reallocate memory to hold the new chunk of data
    *responsePtr = (char*)realloc(*responsePtr, strlen(*responsePtr) + totalSize + 1);
    if (*responsePtr == NULL) {
        // Memory allocation failed
        return 0;
    }

    // Append the new data to the response string
    strncat(*responsePtr, (char*)contents, totalSize);

    return totalSize;
}

// Perform an HTTP/HTTPS request
char* Http_Request(const char* url, HttpMethod method, const char* postData) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return NULL;
    }

    char* response = (char*)calloc(1, sizeof(char));  // Start with an empty string

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (method == HTTP_POST && postData) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        free(response);
        response = NULL;
    }

    curl_easy_cleanup(curl);
    return response;
}

// Start the Cosmic Web Engine
void startEngine() {
    std::cout << "Starting Cosmic Web Engine..." << std::endl;
    // Initialize HTTP library
    if (Http_Init() != 0) {
        std::cout << "Failed to initialize HTTP library." << std::endl;
    }
}

// Example API call function using the HTTP/HTTPS library
void callAPI() {
    // GitHub API endpoint for fetching repository information
    const char* url = "https://api.github.com/repos/octocat/Hello-World";

    // GitHub API requires User-Agent header
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: CosmicWebEngine");

    // Perform the GET request
    char* response = Http_Request(url, HTTP_GET, NULL);

    if (response) {
        std::cout << "API Response: " << std::endl << response << std::endl;
        free(response);  // Free the response buffer
    } else {
        std::cout << "Failed to get response from API." << std::endl;
    }

    // Cleanup HTTP library
    Http_Cleanup();
}
