/****************************************************************************
** Meta object code from reading C++ file 'Host.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/Host.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Host.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Host_t {
    QByteArrayData data[14];
    char stringdata0[138];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Host_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Host_t qt_meta_stringdata_Host = {
    {
QT_MOC_LITERAL(0, 0, 4), // "Host"
QT_MOC_LITERAL(1, 5, 12), // "RequestTitle"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 5), // "title"
QT_MOC_LITERAL(4, 25, 11), // "RequestStop"
QT_MOC_LITERAL(5, 37, 17), // "RequestRenderSize"
QT_MOC_LITERAL(6, 55, 1), // "w"
QT_MOC_LITERAL(7, 57, 1), // "h"
QT_MOC_LITERAL(8, 59, 20), // "UpdateProgressDialog"
QT_MOC_LITERAL(9, 80, 5), // "label"
QT_MOC_LITERAL(10, 86, 8), // "position"
QT_MOC_LITERAL(11, 95, 7), // "maximum"
QT_MOC_LITERAL(12, 103, 18), // "UpdateDisasmDialog"
QT_MOC_LITERAL(13, 122, 15) // "NotifyMapLoaded"

    },
    "Host\0RequestTitle\0\0title\0RequestStop\0"
    "RequestRenderSize\0w\0h\0UpdateProgressDialog\0"
    "label\0position\0maximum\0UpdateDisasmDialog\0"
    "NotifyMapLoaded"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Host[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    0,   47,    2, 0x06 /* Public */,
       5,    2,   48,    2, 0x06 /* Public */,
       8,    3,   53,    2, 0x06 /* Public */,
      12,    0,   60,    2, 0x06 /* Public */,
      13,    0,   61,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    9,   10,   11,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Host::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Host *_t = static_cast<Host *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->RequestTitle((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->RequestStop(); break;
        case 2: _t->RequestRenderSize((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->UpdateProgressDialog((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->UpdateDisasmDialog(); break;
        case 5: _t->NotifyMapLoaded(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (Host::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::RequestTitle)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (Host::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::RequestStop)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (Host::*_t)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::RequestRenderSize)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (Host::*_t)(QString , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::UpdateProgressDialog)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (Host::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::UpdateDisasmDialog)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (Host::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Host::NotifyMapLoaded)) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject Host::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Host.data,
      qt_meta_data_Host,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Host::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Host::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Host.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Host::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void Host::RequestTitle(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Host::RequestStop()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Host::RequestRenderSize(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Host::UpdateProgressDialog(QString _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Host::UpdateDisasmDialog()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Host::NotifyMapLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
