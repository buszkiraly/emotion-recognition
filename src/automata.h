#ifndef AUTOMATA_H
#define AUTOMATA_H

#include "QString"


class Automata
{
    static QString  lookedAt;
    static QString  selected;
    static bool     connected;
    static int      rawhid;


private:
    explicit Automata();
    static bool send(QString);  // sends the given frame to Teensy
    static bool sendLookedAt(); // sends to the machine the lookedAt slot's name
    static bool sendSelected(); // sends to the machine the selected slot's name
public:
    static void Init(); // initialising the machine
    static bool setLookedAt(double, double); // changes the lookedAt slot and calls sendLookedAt();
    static QString getLookedAt(); // returns the lookedAt slot's name
    static bool setSelected(double, double); // changes the selected slot and calls sendSelected();
    static bool setSelected(QString); // changes the selected slot and calls sendSelected();
    static QString getSelected(); // returns the selected slot's name
    static bool getChocolate(); // sends to the machine the selected slot's name
    static QString translateCoord(double,double); // translates relative coordinates( [0,1] ) to slot names
    static void close();

    ~Automata();
    
};

#endif // AUTOMATA_H
