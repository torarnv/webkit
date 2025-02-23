/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2008, 2010, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "ElementAttributeData.h"

#include "Attr.h"
#include "CSSStyleSheet.h"
#include "StyledElement.h"

namespace WebCore {

typedef HashMap<pair<Element*, QualifiedName>, RefPtr<Attr> > AttrMap;
static AttrMap& attrMap()
{
    DEFINE_STATIC_LOCAL(AttrMap, map, ());
    return map;
}

PassRefPtr<Attr> ElementAttributeData::attrIfExists(Element* element, const QualifiedName& name)
{
    if (!m_attrCount)
        return 0;
    return attrMap().get(std::make_pair(element, name)).get();
}

PassRefPtr<Attr> ElementAttributeData::ensureAttr(Element* element, const QualifiedName& name)
{
    AttrMap::AddResult result = attrMap().add(std::make_pair(element, name), 0);
    if (result.isNewEntry) {
        result.iterator->second = Attr::create(element, name);
        ++m_attrCount;
    }
    return result.iterator->second.get();
}

void ElementAttributeData::setAttr(Element* element, const QualifiedName& name, Attr* attr)
{
    ASSERT(!attrMap().contains(std::make_pair(element, name)));
    attrMap().add(std::make_pair(element, name), attr);
    attr->attachToElement(element);
    ++m_attrCount;
}

void ElementAttributeData::removeAttr(Element* element, const QualifiedName& name)
{
    ASSERT(attrMap().contains(std::make_pair(element, name)));
    attrMap().remove(std::make_pair(element, name));
    --m_attrCount;
}

void AttributeVector::removeAttribute(const QualifiedName& name)
{
    size_t index = getAttributeItemIndex(name);
    if (index == notFound)
        return;

    remove(index);
}

ElementAttributeData::~ElementAttributeData()
{
}

void ElementAttributeData::setClass(const String& className, bool shouldFoldCase)
{
    m_classNames.set(className, shouldFoldCase);
}
    
StylePropertySet* ElementAttributeData::ensureInlineStyle(StyledElement* element)
{
    if (!m_inlineStyleDecl) {
        ASSERT(element->isStyledElement());
        m_inlineStyleDecl = StylePropertySet::create();
        m_inlineStyleDecl->setCSSParserMode(strictToCSSParserMode(element->isHTMLElement() && !element->document()->inQuirksMode()));
    }
    return m_inlineStyleDecl.get();
}

StylePropertySet* ElementAttributeData::ensureMutableInlineStyle(StyledElement* element)
{
    if (m_inlineStyleDecl && !m_inlineStyleDecl->isMutable()) {
        m_inlineStyleDecl = m_inlineStyleDecl->copy();
        return m_inlineStyleDecl.get();
    }
    return ensureInlineStyle(element);
}
    
void ElementAttributeData::updateInlineStyleAvoidingMutation(StyledElement* element, const String& text)
{
    // We reconstruct the property set instead of mutating if there is no CSSOM wrapper.
    // This makes wrapperless property sets immutable and so cacheable.
    if (m_inlineStyleDecl && !m_inlineStyleDecl->isMutable())
        m_inlineStyleDecl.clear();
    if (!m_inlineStyleDecl) {
        m_inlineStyleDecl = StylePropertySet::create();
        m_inlineStyleDecl->setCSSParserMode(strictToCSSParserMode(element->isHTMLElement() && !element->document()->inQuirksMode()));
    }
    m_inlineStyleDecl->parseDeclaration(text, element->document()->elementSheet()->internal());
}

void ElementAttributeData::destroyInlineStyle(StyledElement* element)
{
    if (!m_inlineStyleDecl)
        return;
    m_inlineStyleDecl->clearParentElement(element);
    m_inlineStyleDecl = 0;
}

void ElementAttributeData::addAttribute(const Attribute& attribute, Element* element, EInUpdateStyleAttribute inUpdateStyleAttribute)
{
    if (element && inUpdateStyleAttribute == NotInUpdateStyleAttribute)
        element->willModifyAttribute(attribute.name(), nullAtom, attribute.value());

    m_attributes.append(attribute);

    if (element && inUpdateStyleAttribute == NotInUpdateStyleAttribute)
        element->didAddAttribute(const_cast<Attribute*>(&attribute));
}

void ElementAttributeData::removeAttribute(size_t index, Element* element, EInUpdateStyleAttribute inUpdateStyleAttribute)
{
    ASSERT(index < length());

    Attribute& attribute = m_attributes[index];
    QualifiedName name = attribute.name();

    if (element && inUpdateStyleAttribute == NotInUpdateStyleAttribute)
        element->willRemoveAttribute(name, attribute.value());

    if (RefPtr<Attr> attr = attrIfExists(element, name))
        attr->detachFromElementWithValue(attribute.value());

    m_attributes.remove(index);

    if (element && inUpdateStyleAttribute == NotInUpdateStyleAttribute)
        element->didRemoveAttribute(name);
}

bool ElementAttributeData::isEquivalent(const ElementAttributeData* other) const
{
    if (!other)
        return isEmpty();

    unsigned len = length();
    if (len != other->length())
        return false;

    for (unsigned i = 0; i < len; i++) {
        Attribute* otherAttr = other->getAttributeItem(m_attributes[i].name());
        if (!otherAttr || m_attributes[i].value() != otherAttr->value())
            return false;
    }

    return true;
}

void ElementAttributeData::detachAttributesFromElement(Element* element)
{
    if (!m_attrCount)
        return;

    for (unsigned i = 0; i < m_attributes.size(); ++i) {
        if (RefPtr<Attr> attr = attrIfExists(element, m_attributes[i].name()))
            attr->detachFromElementWithValue(m_attributes[i].value());
    }
}

size_t ElementAttributeData::getAttributeItemIndexSlowCase(const String& name, bool shouldIgnoreAttributeCase) const
{
    // Continue to checking case-insensitively and/or full namespaced names if necessary:
    for (unsigned i = 0; i < m_attributes.size(); ++i) {
        if (!m_attributes[i].name().hasPrefix()) {
            if (shouldIgnoreAttributeCase && equalIgnoringCase(name, m_attributes[i].localName()))
                return i;
        } else {
            // FIXME: Would be faster to do this comparison without calling toString, which
            // generates a temporary string by concatenation. But this branch is only reached
            // if the attribute name has a prefix, which is rare in HTML.
            if (equalPossiblyIgnoringCase(name, m_attributes[i].name().toString(), shouldIgnoreAttributeCase))
                return i;
        }
    }
    return notFound;
}

void ElementAttributeData::setAttributes(const ElementAttributeData& other, Element* element)
{
    ASSERT(element);

    // If assigning the map changes the id attribute, we need to call
    // updateId.
    Attribute* oldId = getAttributeItem(element->document()->idAttributeName());
    Attribute* newId = other.getAttributeItem(element->document()->idAttributeName());

    if (oldId || newId)
        element->updateId(oldId ? oldId->value() : nullAtom, newId ? newId->value() : nullAtom);

    Attribute* oldName = getAttributeItem(HTMLNames::nameAttr);
    Attribute* newName = other.getAttributeItem(HTMLNames::nameAttr);

    if (oldName || newName)
        element->updateName(oldName ? oldName->value() : nullAtom, newName ? newName->value() : nullAtom);

    clearAttributes(element);
    m_attributes = other.m_attributes;
    for (unsigned i = 0; i < m_attributes.size(); ++i)
        element->attributeChanged(&m_attributes[i]);
}

void ElementAttributeData::clearAttributes(Element* element)
{
    clearClass();
    detachAttributesFromElement(element);
    m_attributes.clear();
}

void ElementAttributeData::replaceAttribute(size_t index, const Attribute& attribute, Element* element)
{
    ASSERT(element);
    ASSERT(index < length());

    element->willModifyAttribute(attribute.name(), m_attributes[index].value(), attribute.value());
    m_attributes[index] = attribute;
    element->didModifyAttribute(const_cast<Attribute*>(&attribute));
}

PassRefPtr<Attr> ElementAttributeData::getAttributeNode(const String& name, bool shouldIgnoreAttributeCase, Element* element) const
{
    ASSERT(element);
    Attribute* attribute = getAttributeItem(name, shouldIgnoreAttributeCase);
    if (!attribute)
        return 0;
    return const_cast<ElementAttributeData*>(this)->ensureAttr(element, attribute->name());
}

PassRefPtr<Attr> ElementAttributeData::getAttributeNode(const QualifiedName& name, Element* element) const
{
    ASSERT(element);
    Attribute* attribute = getAttributeItem(name);
    if (!attribute)
        return 0;
    return const_cast<ElementAttributeData*>(this)->ensureAttr(element, attribute->name());
}


}
