#include <iostream>
#include <iomanip>
#include "command.hpp"
#include "tuesdayDiscount.hpp"
#include "packageReductionDiscount.hpp"

int main()
{
    Client client1;
    client1.id = 1;
    client1.name = "John Doe";

    Date mondayDate;
    mondayDate.day = Monday;

    Date tuesdayDate;
    tuesdayDate.day = Tuesday;

    Article article1;
    article1.name = "Laptop";
    article1.price = 800.0;
    article1.quantity = 1;

    Article article2;
    article2.name = "Mouse";
    article2.price = 25.0;
    article2.quantity = 2;

    Article article3;
    article3.name = "Keyboard";
    article3.price = 75.0;
    article3.quantity = 1;

    std::cout << std::fixed << std::setprecision(2);

    // Scenario 1: No discount (base Command)
    std::cout << "=== Command 1: No Discount ===" << std::endl;
    Command cmd1(1, mondayDate, client1);
    cmd1.add_article(article1);
    cmd1.add_article(article2);
    std::cout << "Base Price: " << cmd1.compute_base_price() << " EUR" << std::endl;
    std::cout << "Total Price: " << cmd1.get_total_price() << " EUR" << std::endl;
    std::cout << std::endl;

    // Scenario 2: Tuesday discount (10%)
    std::cout << "=== Command 2: Tuesday Discount ===" << std::endl;
    TuesdayDiscountCommand cmd2(2, tuesdayDate, client1);
    cmd2.add_article(article1);
    cmd2.add_article(article2);
    std::cout << "Base Price: " << cmd2.compute_base_price() << " EUR" << std::endl;
    std::cout << "Total Price: " << cmd2.get_total_price() << " EUR" << std::endl;
    std::cout << "Discount Applied: 10%" << std::endl;
    std::cout << std::endl;

    // Scenario 3: Package reduction discount (10 EUR if > 150)
    std::cout << "=== Command 3: Package Reduction Discount ===" << std::endl;
    PackageReductionDiscountCommand cmd3(3, mondayDate, client1);
    cmd3.add_article(article1);
    cmd3.add_article(article2);
    cmd3.add_article(article3);
    std::cout << "Base Price: " << cmd3.compute_base_price() << " EUR" << std::endl;
    std::cout << "Total Price: " << cmd3.get_total_price() << " EUR" << std::endl;
    std::cout << "Discount Applied: 10 EUR (base > 150 EUR)" << std::endl;
    std::cout << std::endl;

    // Scenario 4: Package reduction with price below threshold
    std::cout << "=== Command 4: Package Reduction (Below Threshold) ===" << std::endl;
    PackageReductionDiscountCommand cmd4(4, mondayDate, client1);
    cmd4.add_article(article2);
    cmd4.add_article(article3);
    std::cout << "Base Price: " << cmd4.compute_base_price() << " EUR" << std::endl;
    std::cout << "Total Price: " << cmd4.get_total_price() << " EUR" << std::endl;
    std::cout << "No Discount: base <= 150 EUR" << std::endl;
    std::cout << std::endl;

    return 0;
}