/****************************************************************************
** Meta object code from reading C++ file 'GameList.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/GameList/GameList.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GameList_t {
    QByteArrayData data[9];
    char stringdata0[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GameList_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GameList_t qt_meta_stringdata_GameList = {
    {
QT_MOC_LITERAL(0, 0, 8), // "GameList"
QT_MOC_LITERAL(1, 9, 12), // "GameSelected"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 11), // "NetPlayHost"
QT_MOC_LITERAL(4, 35, 7), // "game_id"
QT_MOC_LITERAL(5, 43, 16), // "SelectionChanged"
QT_MOC_LITERAL(6, 60, 41), // "std::shared_ptr<const UICommo..."
QT_MOC_LITERAL(7, 102, 9), // "game_file"
QT_MOC_LITERAL(8, 112, 19) // "OpenGeneralSettings"

    },
    "GameList\0GameSelected\0\0NetPlayHost\0"
    "game_id\0SelectionChanged\0"
    "std::shared_ptr<const UICommon::GameFile>\0"
    "game_file\0OpenGeneralSettings"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GameList[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    1,   35,    2, 0x06 /* Public */,
       5,    1,   38,    2, 0x06 /* Public */,
       8,    0,   41,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

       0        // eod
};

void GameList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GameList *_t = static_cast<GameList *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->GameSelected(); break;
        case 1: _t->NetPlayHost((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->SelectionChanged((*reinterpret_cast< std::shared_ptr<const UICommon::GameFile>(*)>(_a[1]))); break;
        case 3: _t->OpenGeneralSettings(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (GameList::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameList::GameSelected)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (GameList::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameList::NetPlayHost)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (GameList::*_t)(std::shared_ptr<const UICommon::GameFile> );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameList::SelectionChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (GameList::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameList::OpenGeneralSettings)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject GameList::staticMetaObject = {
    { &QStackedWidget::staticMetaObject, qt_meta_stringdata_GameList.data,
      qt_meta_data_GameList,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *GameList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameList::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GameList.stringdata0))
        return static_cast<void*>(this);
    return QStackedWidget::qt_metacast(_clname);
}

int GameList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStackedWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void GameList::GameSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GameList::NetPlayHost(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GameList::SelectionChanged(std::shared_ptr<const UICommon::GameFile> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void GameList::OpenGeneralSettings()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
