#include "circularindicatormanager.hpp"

namespace caelestia::controls {

CircularIndicatorManager::CircularIndicatorManager(QObject *parent)
    : QObject(parent)
{
}

int CircularIndicatorManager::indeterminateAnimationType() const
{
    return m_indeterminateAnimationType;
}

void CircularIndicatorManager::setIndeterminateAnimationType(int type)
{
    if (m_indeterminateAnimationType == type)
        return;
    m_indeterminateAnimationType = type;
    emit indeterminateAnimationTypeChanged();
    updateDerivedProperties();
}

qreal CircularIndicatorManager::progress() const
{
    return m_progress;
}

void CircularIndicatorManager::setProgress(qreal progress)
{
    if (qFuzzyCompare(m_progress, progress))
        return;
    m_progress = progress;
    emit progressChanged();
    updateDerivedProperties();
}

qreal CircularIndicatorManager::rotation() const
{
    return m_rotation;
}

qreal CircularIndicatorManager::startFraction() const
{
    return m_startFraction;
}

qreal CircularIndicatorManager::endFraction() const
{
    return m_endFraction;
}

qreal CircularIndicatorManager::duration() const
{
    return kDuration;
}

qreal CircularIndicatorManager::completeEndProgress() const
{
    return m_completeEndProgress;
}

void CircularIndicatorManager::setCompleteEndProgress(qreal progress)
{
    if (qFuzzyCompare(m_completeEndProgress, progress))
        return;
    m_completeEndProgress = progress;
    emit completeEndProgressChanged();
    updateDerivedProperties();
}

qreal CircularIndicatorManager::completeEndDuration() const
{
    return kCompleteEndDuration;
}

void CircularIndicatorManager::updateDerivedProperties()
{
    const qreal p = m_progress;
    const qreal tail = m_completeEndProgress;

    qreal newRotation = 0;
    qreal newStart = 0;
    qreal newEnd = 0;

    if (m_indeterminateAnimationType == Advance) {
        // Arc grows from small to large, then resets
        const qreal head = p;
        const qreal visibleEnd = head - tail * 0.1;
        const qreal visibleStart = head - tail * 0.9;

        newStart = qMax(0.0, visibleStart);
        newEnd = qMin(1.0, qMax(newStart + 0.1, visibleEnd));
        newRotation = head * 360.0;
    } else {
        // Retreat: arc shrinks from large to small, then resets
        const qreal head = 1.0 - p;
        const qreal visibleEnd = head + tail * 0.9;
        const qreal visibleStart = head + tail * 0.1;

        newStart = qMax(0.0, visibleStart);
        newEnd = qMin(1.0, qMax(newStart + 0.1, visibleEnd));
        newRotation = head * 360.0;
    }

    bool changed = false;
    if (!qFuzzyCompare(m_rotation, newRotation)) {
        m_rotation = newRotation;
        emit rotationChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_startFraction, newStart)) {
        m_startFraction = newStart;
        emit startFractionChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_endFraction, newEnd)) {
        m_endFraction = newEnd;
        emit endFractionChanged();
    }
}

}
