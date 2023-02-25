#ifndef __CACHE_H__
#define __CACHE_H__

#include <string>
#include <unordered_map>
#include <mutex>

#include "response.hpp"

class Cache
{
private:
    class Node
    {
    public:
        std::string uri;
        std::string response;
        Node *next;
        Node *prev;

        Node() : next(NULL), prev(NULL) {}
        Node(std::string key, std::string response) : uri(key), response(response), next(NULL), prev(NULL) {}
    };
    Node *head;
    Node *tail;

public:
    int capacity;
    int size;
    std::unordered_map<std::string, Node *> cache_map;

    Cache(int capacity) : capacity(capacity), size(0), head(new Node()), tail(new Node())
    {
        head->next = tail;
        tail->prev = head;
    }
    ~Cache()
    {
        Node *curNode = head;
        while (curNode)
        {
            head = head->next;
            delete curNode;
            curNode = head;
        }
    }
    std::string getResponse(std::string uri);
    std::string put(std::string uri, std::string response);
    void addToHead(Node *new_node);
    void removeNodeFromList(Node *node);
    void moveToHead(Node *node);
    std::string deleteTail();
};

#endif
