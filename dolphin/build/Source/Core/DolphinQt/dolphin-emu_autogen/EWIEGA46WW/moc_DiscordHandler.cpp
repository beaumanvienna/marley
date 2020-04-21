/****************************************************************************
** Meta object code from reading C++ file 'DiscordHandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/DiscordHandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DiscordHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DiscordHandler_t {
    QByteArrayData data[8];
    char stringdata0[67];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DiscordHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DiscordHandler_t qt_meta_stringdata_DiscordHandler = {
    {
QT_MOC_LITERAL(0, 0, 14), // "DiscordHandler"
QT_MOC_LITERAL(1, 15, 4), // "Join"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11), // "JoinRequest"
QT_MOC_LITERAL(4, 33, 11), // "std::string"
QT_MOC_LITERAL(5, 45, 2), // "id"
QT_MOC_LITERAL(6, 48, 11), // "discord_tag"
QT_MOC_LITERAL(7, 60, 6) // "avatar"

    },
    "DiscordHandler\0Join\0\0JoinRequest\0"
    "std::string\0id\0discord_tag\0avatar"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DiscordHandler[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,
       3,    3,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 4, 0x80000000 | 4,    5,    6,    7,

       0        // eod
};

void DiscordHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DiscordHandler *_t = static_cast<DiscordHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Join(); break;
        case 1: _t->JoinRequest((*reinterpret_cast< const std::string(*)>(_a[1])),(*reinterpret_cast< const std::string(*)>(_a[2])),(*reinterpret_cast< const std::string(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (DiscordHandler::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DiscordHandler::Join)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DiscordHandler::*_t)(const std::string , const std::string , const std::string );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DiscordHandler::JoinRequest)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject DiscordHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DiscordHandler.data,
      qt_meta_data_DiscordHandler,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *DiscordHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DiscordHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DiscordHandler.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Discord::Handler"))
        return static_cast< Discord::Handler*>(this);
    return QObject::qt_metacast(_clname);
}

int DiscordHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DiscordHandler::Join()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DiscordHandler::JoinRequest(const std::string _t1, const std::string _t2, const std::string _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
