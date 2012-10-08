#ifndef EMOSPECTRUM_H
#define EMOSPECTRUM_H

#include <QWidget>
#include <QLabel>

class EmoSpectrum;

class EmoSpectrum : public QLabel
{
    Q_OBJECT
    std::vector<int>    spectrumVolumes;
    
public:
    explicit EmoSpectrum(QWidget *parent = 0);
    ~EmoSpectrum();
    
private:

public slots:
    void paintEvent(QPaintEvent* event);
    void volumes(std::vector<int> volumes);
};

#endif // EMOSPECTRUM_H
