#ifndef PACKAGEREDUCTIONDISCOUNT_HPP
#define PACKAGEREDUCTIONDISCOUNT_HPP

#include "command.hpp"

class PackageReductionDiscountCommand : public Command
{
public:
    PackageReductionDiscountCommand(int id, Date date, Client client)
        : Command(id, date, client)
    {
    }

    virtual double get_total_price() const
    {
        double basePrice = Command::compute_base_price();
        
        if (basePrice > 150.0)
        {
            return basePrice - 10.0;
        }
        
        return basePrice;
    }
};

#endif