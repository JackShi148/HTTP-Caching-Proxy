#include "cache.h"

std::mutex cache_lock;

std::string Cache::getResponse(std::string uri) {
    std::lock_guard<std::mutex> lck(cache_lock);
    if(this->cache_map.count(uri) == 0) {
        return "";
    }
    Node * response_node = this->cache_map[uri];
    return response_node->response;
}

void Cache::addToHead(Node * new_node) {
    new_node->next = this->head->next;
    this->head->next->prev = new_node;
    new_node->prev = this->head;
    this->head->next = new_node;
}
// remove from list but not to delete
void Cache::removeNodeFromList(Node * node) {
    if(node != this->head && node != this->tail) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        node->prev = NULL;
        node->next = NULL;
    }
}

void Cache::deleteTail() {
    if(this->size > 0) {
        Node * delete_node = this->tail->prev;
        removeNodeFromList(delete_node);
        this->cache_map.erase(delete_node->uri);
        delete delete_node;
    }
}

void Cache::moveToHead(Node * node) {
    removeNodeFromList(node);
    addToHead(node);
}

void Cache::put(std::string uri, std::string response) {
    std::lock_guard<std::mutex> lck(cache_lock);
    // already exist
    if(this->cache_map.count(uri) != 0) {
        this->cache_map[uri]->response = response;
        moveToHead(this->cache_map[uri]);
    }
    // do not exist
    else {
        if(this->size == this->capacity) {
            deleteTail();
            this->size--;
        }
        Node * new_node = new Node(uri, response);
        this->cache_map[uri] = new_node;
        addToHead(new_node);
        this->size++;
    }
}
