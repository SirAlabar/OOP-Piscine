#ifndef TUESDAYDISCOUNT_HPP
#define TUESDAYDISCOUNT_HPP

#include "command.hpp"

class TuesdayDiscountCommand : public Command
{
public:
    TuesdayDiscountCommand(int id, Date date, Client client)
        : Command(id, date, client)
    {
    }

    virtual double get_total_price() const
    {
        double basePrice = Command::compute_base_price();
        
        if (date.day == Tuesday)
        {
            return basePrice * 0.9;
        }
        
        return basePrice;
    }
};

#endif