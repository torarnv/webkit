/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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
 */

#include "config.h"

#if ENABLE(SVG)
#include "SVGAnimatedString.h"

#include "SVGAnimateElement.h"

namespace WebCore {

SVGAnimatedStringAnimator::SVGAnimatedStringAnimator(SVGAnimationElement* animationElement, SVGElement* contextElement)
    : SVGAnimatedTypeAnimator(AnimatedString, animationElement, contextElement)
{
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedStringAnimator::constructFromString(const String& string)
{
    OwnPtr<SVGAnimatedType> animatedType = SVGAnimatedType::createString(new String);
    animatedType->string() = string;
    return animatedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedStringAnimator::startAnimValAnimation(const Vector<SVGAnimatedProperty*>& properties)
{
    return SVGAnimatedType::createString(constructFromBaseValue<SVGAnimatedString>(properties));
}

void SVGAnimatedStringAnimator::stopAnimValAnimation(const Vector<SVGAnimatedProperty*>& properties)
{
    stopAnimValAnimationForType<SVGAnimatedString>(properties);
}

void SVGAnimatedStringAnimator::resetAnimValToBaseVal(const Vector<SVGAnimatedProperty*>& properties, SVGAnimatedType* type)
{
    resetFromBaseValue<SVGAnimatedString>(properties, type, &SVGAnimatedType::string);
}

void SVGAnimatedStringAnimator::animValWillChange(const Vector<SVGAnimatedProperty*>& properties)
{
    animValWillChangeForType<SVGAnimatedString>(properties);
}

void SVGAnimatedStringAnimator::animValDidChange(const Vector<SVGAnimatedProperty*>& properties)
{
    animValDidChangeForType<SVGAnimatedString>(properties);
}

void SVGAnimatedStringAnimator::addAnimatedTypes(SVGAnimatedType*, SVGAnimatedType*)
{
    ASSERT_NOT_REACHED();
}

static String parseStringFromString(SVGAnimationElement*, const String& string)
{
    return string;
}

void SVGAnimatedStringAnimator::calculateAnimatedValue(float percentage, unsigned, OwnPtr<SVGAnimatedType>& from, OwnPtr<SVGAnimatedType>& to, OwnPtr<SVGAnimatedType>& animated)
{
    ASSERT(m_animationElement);
    ASSERT(m_contextElement);

    String& fromString = from->string();
    String& toString = to->string();
    String& animatedString = animated->string();
    m_animationElement->adjustFromToValues<String>(parseStringFromString, fromString, toString, animatedString, percentage, m_contextElement);

    m_animationElement->animateDiscreteType<String>(percentage, fromString, toString, animatedString);
}

float SVGAnimatedStringAnimator::calculateDistance(const String&, const String&)
{
    // No paced animations for strings.
    return -1;
}

}

#endif // ENABLE(SVG)
