/****************************************************************************
** Meta object code from reading C++ file 'MemoryViewWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/Debugger/MemoryViewWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MemoryViewWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MemoryViewWidget_t {
    QByteArrayData data[6];
    char stringdata0[58];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MemoryViewWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MemoryViewWidget_t qt_meta_stringdata_MemoryViewWidget = {
    {
QT_MOC_LITERAL(0, 0, 16), // "MemoryViewWidget"
QT_MOC_LITERAL(1, 17, 18), // "BreakpointsChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 8), // "ShowCode"
QT_MOC_LITERAL(4, 46, 3), // "u32"
QT_MOC_LITERAL(5, 50, 7) // "address"

    },
    "MemoryViewWidget\0BreakpointsChanged\0"
    "\0ShowCode\0u32\0address"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MemoryViewWidget[] = {

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
       3,    1,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,

       0        // eod
};

void MemoryViewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MemoryViewWidget *_t = static_cast<MemoryViewWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->BreakpointsChanged(); break;
        case 1: _t->ShowCode((*reinterpret_cast< u32(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (MemoryViewWidget::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MemoryViewWidget::BreakpointsChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (MemoryViewWidget::*_t)(u32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MemoryViewWidget::ShowCode)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject MemoryViewWidget::staticMetaObject = {
    { &QTableWidget::staticMetaObject, qt_meta_stringdata_MemoryViewWidget.data,
      qt_meta_data_MemoryViewWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *MemoryViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MemoryViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MemoryViewWidget.stringdata0))
        return static_cast<void*>(this);
    return QTableWidget::qt_metacast(_clname);
}

int MemoryViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableWidget::qt_metacall(_c, _id, _a);
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
void MemoryViewWidget::BreakpointsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MemoryViewWidget::ShowCode(u32 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
