#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <vector>
#include <string>

enum DayOfWeek
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

struct Date
{
    DayOfWeek day;
};

struct Client
{
    int id;
    std::string name;
};

struct Article
{
    std::string name;
    double price;
    int quantity;
};

class Command
{
protected:
    int id;
    Date date;
    Client client;
    std::vector<Article> articles;

public:
    Command(int id, Date date, Client client)
        : id(id), date(date), client(client)
    {
    }

    virtual ~Command()
    {
    }

    void add_article(const Article& article)
    {
        articles.push_back(article);
    }

    double compute_base_price() const
    {
        double total = 0.0;
        for (size_t i = 0; i < articles.size(); ++i)
        {
            total += articles[i].price * articles[i].quantity;
        }
        return total;
    }

    virtual double get_total_price() const
    {
        return compute_base_price();
    }
};

#endif