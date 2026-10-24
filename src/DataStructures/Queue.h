#ifndef QUEUE_H
#define QUEUE_H

template <class T>
class Queue {
public:
    struct Node {
        Node *next;
        Node *prev;
        T *data;
    };

private:
    Node *header;
    Node *footer;

    void initialize() {
        header = new Node;
        footer = new Node;
        header->next = footer;
        header->prev = nullptr;
        footer->next = nullptr;
        footer->prev = header;
    }

    void clear() {
        Node *node = header->next;
        while (node != footer) {
            Node *next = node->next;
            delete node->data;
            delete node;
            node = next;
        }
        header->next = footer;
        footer->prev = header;
    }

public:
    Queue() : header(nullptr), footer(nullptr) { initialize(); }

    Queue(const Queue &other) : header(nullptr), footer(nullptr) {
        initialize();
        for (Node *node = other.header->next; node != other.footer; node = node->next)
            enQueue(*node->data);
    }

    virtual ~Queue() {
        clear();
        delete header;
        delete footer;
    }

    virtual void enQueue(const T &value) {
        Node *node = new Node;
        node->data = new T(value);
        node->next = footer;
        node->prev = footer->prev;
        footer->prev->next = node;
        footer->prev = node;
    }

    virtual T *deQueue() {
        if (isEmpty()) return nullptr;
        Node *node = header->next;
        T *copy = new T(*node->data);
        header->next = node->next;
        node->next->prev = header;
        delete node->data;
        delete node;
        return copy;
    }

    Queue &operator=(const Queue &other) {
        if (this != &other) {
            clear();
            for (Node *node = other.header->next; node != other.footer; node = node->next)
                enQueue(*node->data);
        }
        return *this;
    }

    bool isEmpty() const { return header->next == footer; }
    Node *getHeader() const { return header; }
    Node *getFooter() const { return footer; }
};

#endif
