#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>

template <typename T>
class Singleton : public QObject
{
public:
    Singleton(QObject *parent = 0) : QObject(parent) {}
    ~Singleton() {}
    static T & instance()
    {
        static T data;
        return data;
    }
};

typedef Singleton< QMap<QString, QString> > Environ;

#endif
