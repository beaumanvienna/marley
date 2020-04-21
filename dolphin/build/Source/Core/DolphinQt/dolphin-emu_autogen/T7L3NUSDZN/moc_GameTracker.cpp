/****************************************************************************
** Meta object code from reading C++ file 'GameTracker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/GameList/GameTracker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameTracker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GameTracker_t {
    QByteArrayData data[9];
    char stringdata0[112];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GameTracker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GameTracker_t qt_meta_stringdata_GameTracker = {
    {
QT_MOC_LITERAL(0, 0, 11), // "GameTracker"
QT_MOC_LITERAL(1, 12, 10), // "GameLoaded"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 41), // "std::shared_ptr<const UICommo..."
QT_MOC_LITERAL(4, 66, 4), // "game"
QT_MOC_LITERAL(5, 71, 11), // "GameUpdated"
QT_MOC_LITERAL(6, 83, 11), // "GameRemoved"
QT_MOC_LITERAL(7, 95, 11), // "std::string"
QT_MOC_LITERAL(8, 107, 4) // "path"

    },
    "GameTracker\0GameLoaded\0\0"
    "std::shared_ptr<const UICommon::GameFile>\0"
    "game\0GameUpdated\0GameRemoved\0std::string\0"
    "path"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GameTracker[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       5,    1,   32,    2, 0x06 /* Public */,
       6,    1,   35,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void GameTracker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GameTracker *_t = static_cast<GameTracker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->GameLoaded((*reinterpret_cast< const std::shared_ptr<const UICommon::GameFile>(*)>(_a[1]))); break;
        case 1: _t->GameUpdated((*reinterpret_cast< const std::shared_ptr<const UICommon::GameFile>(*)>(_a[1]))); break;
        case 2: _t->GameRemoved((*reinterpret_cast< const std::string(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< std::shared_ptr<const UICommon::GameFile> >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< std::shared_ptr<const UICommon::GameFile> >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< std::string >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (GameTracker::*_t)(const std::shared_ptr<const UICommon::GameFile> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameTracker::GameLoaded)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (GameTracker::*_t)(const std::shared_ptr<const UICommon::GameFile> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameTracker::GameUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (GameTracker::*_t)(const std::string & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameTracker::GameRemoved)) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject GameTracker::staticMetaObject = {
    { &QFileSystemWatcher::staticMetaObject, qt_meta_stringdata_GameTracker.data,
      qt_meta_data_GameTracker,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *GameTracker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameTracker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GameTracker.stringdata0))
        return static_cast<void*>(this);
    return QFileSystemWatcher::qt_metacast(_clname);
}

int GameTracker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFileSystemWatcher::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void GameTracker::GameLoaded(const std::shared_ptr<const UICommon::GameFile> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GameTracker::GameUpdated(const std::shared_ptr<const UICommon::GameFile> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GameTracker::GameRemoved(const std::string & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
