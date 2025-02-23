/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#if ENABLE(Condition1) || ENABLE(Condition2)

#ifndef V8TestInterface_h
#define V8TestInterface_h

#include "TestInterface.h"
#include "V8DOMWrapper.h"
#include "WrapperTypeInfo.h"
#include <v8.h>
#include <wtf/HashMap.h>
#include <wtf/text/StringHash.h>

namespace WebCore {

class V8TestInterface {
public:
    static const bool hasDependentLifetime = true;
    static bool HasInstance(v8::Handle<v8::Value>);
    static v8::Persistent<v8::FunctionTemplate> GetRawTemplate();
    static v8::Persistent<v8::FunctionTemplate> GetTemplate();
    static TestInterface* toNative(v8::Handle<v8::Object> object)
    {
        return reinterpret_cast<TestInterface*>(object->GetPointerFromInternalField(v8DOMWrapperObjectIndex));
    }
    inline static v8::Handle<v8::Object> wrap(TestInterface*, v8::Isolate* = 0);
    static void derefObject(void*);
    static WrapperTypeInfo info;
    static ActiveDOMObject* toActiveDOMObject(v8::Handle<v8::Object>);
    static v8::Handle<v8::Value> constructorCallback(const v8::Arguments&);
    static v8::Handle<v8::Value> namedPropertySetter(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::AccessorInfo&);
    static const int internalFieldCount = v8DefaultWrapperInternalFieldCount + 0;
private:
    static v8::Handle<v8::Object> wrapSlow(PassRefPtr<TestInterface>);
};

v8::Handle<v8::Object> V8TestInterface::wrap(TestInterface* impl, v8::Isolate* isolate)
{
        v8::Handle<v8::Object> wrapper = getActiveDOMObjectMap().get(impl);
        if (!wrapper.IsEmpty())
            return wrapper;
    return V8TestInterface::wrapSlow(impl);
}

inline v8::Handle<v8::Value> toV8(TestInterface* impl, v8::Isolate* isolate = 0)
{
    if (!impl)
        return v8::Null();
    return V8TestInterface::wrap(impl, isolate);
}
inline v8::Handle<v8::Value> toV8(PassRefPtr< TestInterface > impl, v8::Isolate* isolate = 0)
{
    return toV8(impl.get(), isolate);
}

}

#endif // V8TestInterface_h
#endif // ENABLE(Condition1) || ENABLE(Condition2)

