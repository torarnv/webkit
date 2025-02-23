/*
 * Copyright (C) Research In Motion Limited 2011, 2012. All rights reserved.
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
#include "SVGAnimatedPointList.h"

#include "SVGAnimateElement.h"
#include "SVGParserUtilities.h"
#include "SVGPointList.h"

namespace WebCore {

SVGAnimatedPointListAnimator::SVGAnimatedPointListAnimator(SVGAnimationElement* animationElement, SVGElement* contextElement)
    : SVGAnimatedTypeAnimator(AnimatedPoints, animationElement, contextElement)
{
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedPointListAnimator::constructFromString(const String& string)
{
    OwnPtr<SVGAnimatedType> animtedType = SVGAnimatedType::createPointList(new SVGPointList);
    pointsListFromSVGData(animtedType->pointList(), string);
    return animtedType.release();
}

PassOwnPtr<SVGAnimatedType> SVGAnimatedPointListAnimator::startAnimValAnimation(const Vector<SVGAnimatedProperty*>& properties)
{
    return SVGAnimatedType::createPointList(constructFromBaseValue<SVGAnimatedPointList>(properties));
}

void SVGAnimatedPointListAnimator::stopAnimValAnimation(const Vector<SVGAnimatedProperty*>& properties)
{
    stopAnimValAnimationForType<SVGAnimatedPointList>(properties);
}

void SVGAnimatedPointListAnimator::resetAnimValToBaseVal(const Vector<SVGAnimatedProperty*>& properties, SVGAnimatedType* type)
{
    resetFromBaseValue<SVGAnimatedPointList>(properties, type, &SVGAnimatedType::pointList);
}

void SVGAnimatedPointListAnimator::animValWillChange(const Vector<SVGAnimatedProperty*>& properties)
{
    animValWillChangeForType<SVGAnimatedPointList>(properties);
}

void SVGAnimatedPointListAnimator::animValDidChange(const Vector<SVGAnimatedProperty*>& properties)
{
    animValDidChangeForType<SVGAnimatedPointList>(properties);
}

void SVGAnimatedPointListAnimator::addAnimatedTypes(SVGAnimatedType* from, SVGAnimatedType* to)
{
    ASSERT(from->type() == AnimatedPoints);
    ASSERT(from->type() == to->type());

    SVGPointList& fromPointList = from->pointList();
    SVGPointList& toPointList = to->pointList();
    unsigned itemCount = fromPointList.size();
    if (!itemCount || itemCount != toPointList.size())
        return;
    for (unsigned n = 0; n < itemCount; ++n) {
        FloatPoint& to = toPointList.at(n);
        to += fromPointList.at(n);
    }
}

void SVGAnimatedPointListAnimator::calculateAnimatedValue(float percentage, unsigned, OwnPtr<SVGAnimatedType>& from, OwnPtr<SVGAnimatedType>& to, OwnPtr<SVGAnimatedType>& animated)
{
    ASSERT(m_animationElement);
    ASSERT(m_contextElement);
    
    SVGPointList& fromPointList = from->pointList();
    SVGPointList& toPointList = to->pointList();
    SVGPointList& animatedPointList = animated->pointList();
    if (!m_animationElement->adjustFromToListValues<SVGPointList>(0, fromPointList, toPointList, animatedPointList, percentage, m_contextElement))
        return;

    if (!percentage)
        animatedPointList = fromPointList;
    else if (percentage == 1)
        animatedPointList = toPointList;
    else {
        animatedPointList.clear();
        if (!fromPointList.isEmpty() && !toPointList.isEmpty())
            SVGPointList::createAnimated(fromPointList, toPointList, animatedPointList, percentage);
            
        // Fall back to discrete animation if the points are not compatible
        AnimationMode animationMode = m_animationElement->animationMode();
        if (animatedPointList.isEmpty())
            animatedPointList = ((animationMode == FromToAnimation && percentage > 0.5) || animationMode == ToAnimation || percentage == 1) 
            ? toPointList : fromPointList;
    }
}

float SVGAnimatedPointListAnimator::calculateDistance(const String&, const String&)
{
    // FIXME: Distance calculation is not possible for SVGPointList right now. We need the distance of for every single value.
    return -1;
}

}

#endif // ENABLE(SVG)
