#ifndef HELPER_H
#define HELPER_H

#include <ctime>
#include <string>
#include <sstream>
#include <fstream>

namespace Helper
{
    template <class T>

    std::string ToString(const T &);

    struct DateTime
    {
        DateTime ()
        {
            time_t ms;
            time (&ms);

            struct tm *info = localtime(&ms);

            D = info->tm_mday;
            //month starts at 0 not 1
            m = info->tm_mon + 1;
            y = info->tm_year + 1900;
            M = info->tm_min;
            H = info->tm_hour;
            S = info->tm_sec;
        }

        DateTime(int D, int m, int y, int H, int M, int S): D(D), m(m), y(y), H(H), M(M), S(S){}
        DateTime(int D, int m, int y): D(D), m(m), y(y){}


        DateTime Now() const
        {
            return DateTime();
        }
        int D, m, y, H, M, S;

        std::string GetDateString() const
        {
            //DD.mm.YYYY formatting
            return std::string( D < 10 ? "0" : "") + ToString(D) + std::string( m < 10 ? ".0" : "") + ToString(m) + "." + ToString(y);
        }

        std::string GetTimeString(const std::string &sep = ":") const
        {
            return std::string(H < 10 ? "0" : "") + ToString(H) + sep +std::string( M < 10 ? "0" : "")+ ToString(M) + sep + std::string( S<10 ? "0" : "") + ToString (S);
        }

        std::string  GetDateTimeString(const std::string &sep = ":") const
        {
            return GetDateString() + "_" + GetTimeString(sep);
        }


    };

    template <class T>

    std::string ToString(const T &e)
    {
        std::ostringstream s;
        s << e;
        return s.str();
    }


    void WriteAppLog (const std::string &s)
    {
        std::ofstream file("AppLog.txt", std::ios::app);
        file << "[" << Helper::DateTime().GetDateTimeString() << "]" << "\n" << s << std::endl << "\n";
        file.close();
    }
}



#endif // HELPER_H
