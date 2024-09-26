#define COSMIC_WEB_ENGINE
#define COSMIC_WEB_ENGINE_IMPLEMENTATION
#include "include/engine.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/stb_truetype.h"
#include "include/httplib.h"  // Include the cpp-httplib header
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <string>
#include "include/libcurl/curl/curl.h"
#include <thread>
#include <regex>
#include "include/gumbo.h"
#include "include/Symm/symm.h" // Use the Symm header file 
#define SYMM_API
#define SYMM_IMPLEMENTATION
// Copyright AR-DEV-1
// Copyright The Cosmic Web Authors
// This software and its source is provided under GNU GPL 3.0
// The runtime, software, source and etc are all provided under GNU GPL 3.0

// Cosmic Web Engine Implementation
// Callback function to handle data received by libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Function to perform an HTTP GET request using libcurl
std::string Http_Get(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            readBuffer = "";
        }
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Helper function to extract text content from an HTML node
std::string ExtractTextFromNode(GumboNode* node) {
    if (node->type != GUMBO_NODE_TEXT) return "";

    return std::string(node->v.text.text);
}

// Recursive function to search for results in the HTML tree
void SearchForResults(GumboNode* node, std::string& resultsHtml) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode* child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT && child->v.element.tag == GUMBO_TAG_A) {
            GumboNode* link = static_cast<GumboNode*>(child);
            std::string title = ExtractTextFromNode(link);
            std::string url = ExtractTextFromNode(link);
            if (!url.empty() && !title.empty()) {
                resultsHtml += "<div class='result'>";
                resultsHtml += "<h3><a href='" + url + "'>" + title + "</a></h3>";
                resultsHtml += "</div>";
            }
        }
        SearchForResults(child, resultsHtml);
    }
}

// Function to handle search requests and parse DuckDuckGo results
void handleSearchRequest(const std::string& query, httplib::Response& res) {
    std::string searchUrl = "https://duckduckgo.com/lite?q=" + curl_easy_escape(nullptr, query.c_str(), query.length());

    // Perform the HTTP GET request
    std::string response = Http_Get(searchUrl);

    if (response.empty()) {
        std::cerr << "Failed to fetch search results." << std::endl;
        res.status = 500;
        res.set_content("Failed to fetch search results.", "text/plain");
        return;
    }

    // Parse HTML with Gumbo
    GumboOutput* output = gumbo_parse(response.c_str());

    std::string resultsHtml;
    SearchForResults(output->root, resultsHtml);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    if (resultsHtml.empty()) {
        resultsHtml = "<p>No results found.</p>";
    }

    res.set_content(resultsHtml, "text/html");
}
