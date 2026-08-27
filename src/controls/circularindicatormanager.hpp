#pragma once

#include <qqmlintegration.h>
#include <qobject.h>
#include <qproperty.h>

namespace caelestia::controls {

class CircularIndicatorManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int indeterminateAnimationType READ indeterminateAnimationType WRITE setIndeterminateAnimationType NOTIFY indeterminateAnimationTypeChanged)
    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(qreal rotation READ rotation NOTIFY rotationChanged)
    Q_PROPERTY(qreal startFraction READ startFraction NOTIFY startFractionChanged)
    Q_PROPERTY(qreal endFraction READ endFraction NOTIFY endFractionChanged)
    Q_PROPERTY(qreal duration READ duration CONSTANT)
    Q_PROPERTY(qreal completeEndProgress READ completeEndProgress WRITE setCompleteEndProgress NOTIFY completeEndProgressChanged)
    Q_PROPERTY(qreal completeEndDuration READ completeEndDuration CONSTANT)

public:
    enum AnimType {
        Advance = 0,
        Retreat
    };
    Q_ENUM(AnimType)

    explicit CircularIndicatorManager(QObject *parent = nullptr);

    [[nodiscard]] int indeterminateAnimationType() const;
    void setIndeterminateAnimationType(int type);

    [[nodiscard]] qreal progress() const;
    void setProgress(qreal progress);

    [[nodiscard]] qreal rotation() const;
    [[nodiscard]] qreal startFraction() const;
    [[nodiscard]] qreal endFraction() const;
    [[nodiscard]] qreal duration() const;
    [[nodiscard]] qreal completeEndProgress() const;
    void setCompleteEndProgress(qreal progress);
    [[nodiscard]] qreal completeEndDuration() const;

signals:
    void indeterminateAnimationTypeChanged();
    void progressChanged();
    void rotationChanged();
    void startFractionChanged();
    void endFractionChanged();
    void completeEndProgressChanged();

private:
    void updateDerivedProperties();

    int m_indeterminateAnimationType = Advance;
    qreal m_progress = 0;
    qreal m_rotation = 0;
    qreal m_startFraction = 0;
    qreal m_endFraction = 0;
    qreal m_completeEndProgress = 0;

    static constexpr qreal kDuration = 1.5;
    static constexpr qreal kCompleteEndDuration = 0.3;
};

}
