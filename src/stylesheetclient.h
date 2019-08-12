#ifndef STYLESHEETCLIENT_H
#define STYLESHEETCLIENT_H

// C/C++ Libraries
#include <iostream>

// Qt Libraries
#include <QObject>
#include <QSharedMemory>
#include <QBuffer>
#include <QDataStream>
#include <QTimer>
#include <QVariant>


using namespace std;


class StyleSheetClient : public QObject
{
    Q_OBJECT
public:
    explicit StyleSheetClient(QObject *parent = 0) : QObject(parent)
    {
        // set variables
        m_app_widgets_set = false;

        // request a style sheet
        int interval = 100;
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(pollNewData()));
        m_timer.start(interval);
    }

    ~StyleSheetClient()
    {
        if(m_aw_memory.isAttached())        m_aw_memory.detach();
        if(m_aw_memory_ready.isAttached())  m_aw_memory_ready.detach();
    }

    // ---------------------------------------------
    QString getStyleSheet()
    {
        return this->readFromSharedMemory("StyleSheetSharedMemory", "").toString();
    }

    bool styleSheetIsAvailable()
    {
        return this->readFromSharedMemory("StyleSheetSharedMemoryReady", false).toBool();
    }

    // ---------------------------------------------
    QString readAppWidgets()
    {
        return this->readFromSharedMemory("AppWidgetsMemory", "No Widgets").toString();
    }

    // ---------------------------------------------
    void writeSharedMemory(QSharedMemory& shared_memory, const QString& key, const QVariant& data)
    {
        // create a buffer and write the data to the buffer
        QBuffer buffer;
        buffer.open( QBuffer::ReadWrite );
        QDataStream out( &buffer );
        out << data;

        int size = buffer.size();

        // assign the key
        shared_memory.setKey(key);

        // attach to shared memory
        if(shared_memory.isAttached())
        {
            if(!shared_memory.detach())
                cout << "Failed to detach memory " << shared_memory.key().toStdString() << endl;
        }

        // create the shared memory
        if(!shared_memory.create(size))
        {
            cout << "Unable to create shared memory segment." << endl;
            return;
        }

        // lock the shared memory
        shared_memory.lock();

        // write into the shared memory
        char *data_ptr = (char*)shared_memory.data(); // pointer to shared memory
        const char *from = buffer.data().data();
        memcpy( data_ptr, from, qMin( shared_memory.size(), size ) );

        // unlock the shared memory
        shared_memory.unlock();

        //shared_memory.detach();
    }

    // ---------------------------------------------
    void updateSharedMemory(const QString& key, const QVariant& data)
    {
        // create a buffer and write the data to the buffer
        QBuffer buffer;
        buffer.open( QBuffer::ReadWrite );
        QDataStream out( &buffer );
        out << data;

        int size = buffer.size();

        // assign the key so we can update the data
        QSharedMemory shared_memory;
        shared_memory.setKey(key);

        // attach to shared memory
        if(!shared_memory.isAttached())
            shared_memory.attach();

        // lock the shared memory
        shared_memory.lock();

        // write into the shared memory
        char *data_ptr = (char*)shared_memory.data(); // pointer to shared memory
        const char *from = buffer.data().data();
        memcpy( data_ptr, from, qMin( shared_memory.size(), size ) );

        // unlock the shared memory
        shared_memory.unlock();

        shared_memory.detach();
    }

    // ---------------------------------------------
    QVariant readFromSharedMemory(const QString& key, QVariant default_value)
    {
        // set shared memory key
        QSharedMemory shared_memory;
        shared_memory.setKey(key);

        // attempt to attach to shared memory segment
        if (!shared_memory.attach())
        {
            // if an attempt of reading from the shared memory before data is written
            //cout << "ERROR: Failed to attach to shared memory " << key.toStdString() << endl;
            return default_value;
        }

        // crate a buffer to read the data in
        QBuffer buffer;
        QDataStream in(&buffer);
        QVariant data;

        // lock the memory
        if(!shared_memory.lock())
            cout << "Failed to lock" << endl;

        // read the data
        //char* data_ptr = (char*)shared_memory.constData();
        buffer.setData((char*)shared_memory.constData(), shared_memory.size());
        buffer.open(QBuffer::ReadOnly);

        in >> data;

        // unlock the memory
        if(!shared_memory.unlock())
            cout << "Failed to unlock" << endl;

        // detach the memory
        shared_memory.detach();

        return data;
    }

signals:
    void styleSheetReady(QString style_sheet);

public slots:
    void pollNewData()
    {
        // write style sheet
        if(this->styleSheetIsAvailable())
        {
            // indicate that the style sheet has been read
            this->updateSharedMemory("StyleSheetSharedMemoryReady", false);

            emit this->styleSheetReady(this->getStyleSheet());
        }

        // write app widgets
        if(!m_app_widgets_set)
        {
            this->writeSharedMemory(m_aw_memory, "AppWidgetsMemory", this->getAppWidgets());
            this->writeSharedMemory(m_aw_memory_ready, "AppWidgetsMemoryReady", true);

            m_app_widgets_set = true;
        }
    }

protected:
    QString getAppWidgets()
    {
        if(this->parent() != Q_NULLPTR)
        {
            QStringList widgets;
            this->getParentObjects(this->parent(), widgets, 1);
            return widgets.join('\n');
        }
        return QString();
    }

    void getParentObjects(QObject* parent, QStringList& objects, const int& depth)
    {
        // write parent
        QString object_name = parent->objectName();
        QString class_name = parent->metaObject()->className();
        objects << QString("%0,%1,%2").arg(depth).arg(object_name).arg(class_name);

        // write the children
        for(QObject* child: parent->children())
        {
            this->getParentObjects(child, objects, depth + 1);
        }
    }

private:
    /** This member variable contains the application widgets shared memory
     */
    QSharedMemory m_aw_memory;

    /** This member variable contains the application widgets ready shared memory
     */
    QSharedMemory m_aw_memory_ready;

    QTimer m_timer;

    bool m_app_widgets_set;
};

#endif // STYLESHEETCLIENT_H
