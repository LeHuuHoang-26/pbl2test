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
    public:
        Queue(): header(new Node), footer(new Node) {
            header->next = footer;
            header->prev = nullptr;

            footer->next = nullptr;
            footer->prev = header;
        }

        Queue(const Queue &Q): header(new Node), footer(new Node) {
            header->next = footer;
            header->prev = nullptr;

            footer->next = nullptr;
            footer->prev = header;

            Node *tmpQ = Q.header->next;
            while (tmpQ != Q.footer) {
                Node *tmp = new Node;
                tmp->data = new T(*tmpQ->data);

                tmp->next = footer;
                tmp->prev = footer->prev;
                footer->prev->next = tmp;
                footer->prev = tmp;

                tmpQ = tmpQ->next;
            }
        }

        ~Queue() {
            Node *tmp = header->next;
            while (tmp != footer) {
                Node *tmpN = tmp->next;
                delete tmp->data;
                delete tmp;

                tmp = tmpN;
            }

            delete header;
            delete footer;
        }

        void enQueue(const T &data) {
            Node *tmp = new Node;
            tmp->data = new T(data);

            tmp->next = footer;
            tmp->prev = footer->prev;
            footer->prev->next = tmp;
            footer->prev = tmp;
        }

        T* deQueue() {
            if (header->next == footer)
                return nullptr;

            Node *tmp = header->next;
            T* tmpData = new T(*tmp->data);
            header->next = tmp->next;
            tmp->next->prev = header;

            delete tmp->data;
            delete tmp;

            return tmpData;
        }

        bool isEmpty() const {
            return (header->next == footer);
        }
};

#endif