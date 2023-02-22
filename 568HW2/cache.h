#ifndef __CACHE_H__
#define __CACHE_H__

#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>

#include "response.hpp"

class Cache {
private:
    class Node {
    public:
        std::string key;
        std::string response;  // 完整的response报文
        Node * next;
        Node * prev;

        Node() : next(NULL), prev(NULL) {}
        Node(std::string key, std::string response) : key(key), response(response), next(NULL), prev(NULL) {}
    };
public:
    int capacity;
    int size;
    std::unordered_map<std::string, Node*> cache_map;
    Node * head;
    Node * tail;

    Cache(int capacity) : capacity(capacity), size(0), head(new Node()), tail(new Node()) {
        head->next = tail;
        tail->prev = head;
    }
    std::string getResponse(std::string key);
    void put(std::string uri, std::string response);
    void addToHead(Node * new_node);
    void removeNodeFromList(Node * node);
    void moveToHead(Node * node);
    void deleteTail();
    void moveToHead(Node * new_node);
    int putResponseToCache(std::string uri, Response response);   // 写到proxy里面？
};

#endif