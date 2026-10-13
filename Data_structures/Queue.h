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
        Node *firstWaiting;
    public:
        Queue(): header(new Node), footer(new Node), firstWaiting(nullptr) {
            header->next = footer;
            header->prev = nullptr;

            footer->next = nullptr;
            footer->prev = header;
        }

        Queue(const Queue &Q): header(new Node), footer(new Node), firstWaiting(nullptr) {
            header->next = footer;
            header->prev = nullptr;

            footer->next = nullptr;
            footer->prev = header;

            Node *tmpQ = Q.header->next;
            while (tmpQ != Q.footer) {
                enQueue(*tmpQ->data);
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
            firstWaiting = nullptr;
        }

        void enQueue(const T &data) {
            Node *tmp = new Node;
            tmp->data = new T(data);

            tmp->next = footer;
            tmp->prev = footer->prev;
            footer->prev->next = tmp;
            footer->prev = tmp;

            if constexpr (requires { tmp->data->getState(); }) {

                if (tmp->data->getState() == 0 && firstWaiting == nullptr) {
                    firstWaiting = tmp;
                }
            } else {

                if (firstWaiting == nullptr) {
                    firstWaiting = tmp;
                }
            }
        }

        Node *updateFirstWaiting() {
            if constexpr (requires { header->next->data->getState(); }) {
                Node *cur = header->next;
                while (cur != footer) {
                    if (cur->data && cur->data->getState() == 0) {
                        firstWaiting = cur;
                        return firstWaiting;
                    }
                    cur = cur->next;
                }
                firstWaiting = nullptr;
                return nullptr;
            } else {
                firstWaiting = (header->next != footer) ? header->next : nullptr;
                return firstWaiting;
            }
        }

        Node *getFirstWaiting() {
            if constexpr (requires { header->next->data->getState(); }) {
                if (firstWaiting != nullptr)
                    return firstWaiting;
                return updateFirstWaiting();
            } else {
                return (header->next != footer) ? header->next : nullptr;
            }
        }

        T* deQueue() {
            if (header->next == footer)
                return nullptr;

            if constexpr (requires { header->next->data->getState(); }) {
                Node *target = getFirstWaiting();
                if (!target)
                    return nullptr;

                target->data->setState(1);

                T *tmpData = new T(*target->data);

                Node *cur = target->next;
                firstWaiting = nullptr;
                while (cur != footer) {
                    if (cur->data && cur->data->getState() == 0) {
                        firstWaiting = cur;
                        break;
                    }
                    cur = cur->next;
                }

                return tmpData;
            } else {
                Node *tmp = header->next;
                T* tmpData = new T(*tmp->data);
                header->next = tmp->next;
                tmp->next->prev = header;

                delete tmp->data;
                delete tmp;

                firstWaiting = (header->next != footer) ? header->next : nullptr;
                return tmpData;
            }
        }

        Queue &operator=(const Queue &Q) {
            if (this != &Q) {
                Node *tmp = header->next;
                while (tmp != footer) {
                    Node *tmpN = tmp->next;
                    delete tmp->data;
                    delete tmp;
                    tmp = tmpN;
                }
                header->next = footer;
                footer->prev = header;
                firstWaiting = nullptr;

                Node *tmpQ = Q.header->next;
                while (tmpQ != Q.footer) {
                    enQueue(*tmpQ->data);
                    tmpQ = tmpQ->next;
                }
            }
            return *this;
        }

        bool isEmpty() const {
            return (header->next == footer);
        }

        Node *getHeader() const {
            return header;
        }

        Node *getFooter() const {
            return footer;
        }
};

#endif
