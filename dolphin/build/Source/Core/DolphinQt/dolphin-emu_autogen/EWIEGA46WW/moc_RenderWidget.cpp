/****************************************************************************
** Meta object code from reading C++ file 'RenderWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/RenderWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RenderWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_RenderWidget_t {
    QByteArrayData data[13];
    char stringdata0[132];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RenderWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RenderWidget_t qt_meta_stringdata_RenderWidget = {
    {
QT_MOC_LITERAL(0, 0, 12), // "RenderWidget"
QT_MOC_LITERAL(1, 13, 13), // "EscapePressed"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 6), // "Closed"
QT_MOC_LITERAL(4, 35, 13), // "HandleChanged"
QT_MOC_LITERAL(5, 49, 6), // "handle"
QT_MOC_LITERAL(6, 56, 12), // "StateChanged"
QT_MOC_LITERAL(7, 69, 10), // "fullscreen"
QT_MOC_LITERAL(8, 80, 11), // "SizeChanged"
QT_MOC_LITERAL(9, 92, 9), // "new_width"
QT_MOC_LITERAL(10, 102, 10), // "new_height"
QT_MOC_LITERAL(11, 113, 12), // "FocusChanged"
QT_MOC_LITERAL(12, 126, 5) // "focus"

    },
    "RenderWidget\0EscapePressed\0\0Closed\0"
    "HandleChanged\0handle\0StateChanged\0"
    "fullscreen\0SizeChanged\0new_width\0"
    "new_height\0FocusChanged\0focus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RenderWidget[] = {

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
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,
       4,    1,   46,    2, 0x06 /* Public */,
       6,    1,   49,    2, 0x06 /* Public */,
       8,    2,   52,    2, 0x06 /* Public */,
      11,    1,   57,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::VoidStar,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    9,   10,
    QMetaType::Void, QMetaType::Bool,   12,

       0        // eod
};

void RenderWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RenderWidget *_t = static_cast<RenderWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->EscapePressed(); break;
        case 1: _t->Closed(); break;
        case 2: _t->HandleChanged((*reinterpret_cast< void*(*)>(_a[1]))); break;
        case 3: _t->StateChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->SizeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->FocusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (RenderWidget::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::EscapePressed)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (RenderWidget::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::Closed)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (RenderWidget::*_t)(void * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::HandleChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (RenderWidget::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::StateChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (RenderWidget::*_t)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::SizeChanged)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (RenderWidget::*_t)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RenderWidget::FocusChanged)) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject RenderWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RenderWidget.data,
      qt_meta_data_RenderWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *RenderWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RenderWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RenderWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RenderWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void RenderWidget::EscapePressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void RenderWidget::Closed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void RenderWidget::HandleChanged(void * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RenderWidget::StateChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void RenderWidget::SizeChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void RenderWidget::FocusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
