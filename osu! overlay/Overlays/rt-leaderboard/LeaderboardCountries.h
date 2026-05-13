#pragma once

#include "Shared/Util/Streams.h"
#include "Helper/StringHelper.h"

#include <string>
#include <vector>
#include <optional>

class LeaderboardCountries
{
public:
    struct Country
    {
        std::wstring code;
        std::wstring name;
    };

    static std::optional<size_t> FindIndexByName(const std::wstring& name)
    {
        for (size_t i = 0; i < COUNTRIES.size(); i++)
        {
            if (COUNTRIES[i].name == name)
                return i;
        }
        return std::nullopt;
    }

    static std::optional<size_t> FindIndexByCode(const std::wstring& code)
    {
        std::wstring codeUpper = to_uppercase(code);
        for (size_t i = 0; i < COUNTRIES.size(); i++)
        {
            if (COUNTRIES[i].code == codeUpper)
                return i;
        }
        return std::nullopt;
    }

    static std::vector<std::wstring> GetCountryNames()
    {
        return streams::From(COUNTRIES)
            .Map<std::wstring>([](const Country& country) { return country.name; })
            .ToVector();
    }

    static std::vector<std::wstring> GetCountryCodes()
    {
        return streams::From(COUNTRIES)
            .Map<std::wstring>([](const Country& country) { return country.code; })
            .ToVector();
    }

    inline static const std::vector<Country> COUNTRIES = {
        { L"AF", L"Afghanistan" },
        { L"AX", L"Aland Islands" },
        { L"AL", L"Albania" },
        { L"DZ", L"Algeria" },
        { L"AS", L"American Samoa" },
        { L"AD", L"Andorra" },
        { L"AO", L"Angola" },
        { L"AI", L"Anguila" },
        { L"AQ", L"Antarctica" },
        { L"AG", L"Antigua and Barbuda" },
        { L"AR", L"Argentina" },
        { L"AM", L"Armenia" },
        { L"AW", L"Aruba" },
        { L"AP", L"Asia/Pacific Region" },
        { L"AU", L"Australia" },
        { L"AT", L"Austria" },
        { L"AZ", L"Azerbaijan" },
        { L"BS", L"Bahamas" },
        { L"BH", L"Bahrain" },
        { L"BD", L"Bangladesh" },
        { L"BB", L"Barbados" },
        { L"BY", L"Belarus" },
        { L"BE", L"Belgium" },
        { L"BZ", L"Belize" },
        { L"BJ", L"Benin" },
        { L"BM", L"Bermuda" },
        { L"BT", L"Bhutan" },
        { L"BO", L"Bolivia" },
        { L"BA", L"Bosnia and Herzegovina" },
        { L"BW", L"Botswana" },
        { L"BR", L"Brazil" },
        { L"BN", L"Brunei" },
        { L"BG", L"Bulgaria" },
        { L"BF", L"Burkina Faso" },
        { L"BI", L"Burundi" },
        { L"CV", L"Cabo Verde" },
        { L"KH", L"Cambodia" },
        { L"CM", L"Cameroon" },
        { L"CA", L"Canada" },
        { L"BQ", L"Carribean Netherlands" },
        { L"KY", L"Cayman Islands" },
        { L"TD", L"Chad" },
        { L"CL", L"Chile" },
        { L"CN", L"China" },
        { L"CX", L"Christmas Island" },
        { L"CO", L"Colombia" },
        { L"KM", L"Camoros" },
        { L"CG", L"Congo" },
        { L"CK", L"Cook Islands" },
        { L"CR", L"Costa Rica" },
        { L"CI", L"Cote D'lvoire" },
        { L"HR", L"Croatia" },
        { L"CU", L"Cuba" },
        { L"CW", L"Curaçao" },
        { L"CY", L"Cyprus" },
        { L"CZ", L"Czechia" },
        { L"DK", L"Denmark" },
        { L"DJ", L"Djibouti" },
        { L"DM", L"Dominica" },
        { L"DO", L"Dominican Republic" },
        { L"EC", L"Ecuador" },
        { L"EG", L"Egypt" },
        { L"SV", L"El Salvador" },
        { L"GQ", L"Equatorial Guinea" },
        { L"ER", L"Eritrea" },
        { L"EE", L"Estonia" },
        { L"SZ", L"Eswatini" },
        { L"ET", L"Ethiopia" },
        { L"EU", L"Europe" },
        { L"FK", L"Falkland Islands (Malvinas)" },
        { L"FO", L"Faroe Islands" },
        { L"FM", L"Federated States of Micronesia" },
        { L"FJ", L"Fiji" },
        { L"FI", L"Finland" },
        { L"FR", L"France" },
        { L"GF", L"French Guiana" },
        { L"PF", L"French Polynesia" },
        { L"GA", L"Gabon" },
        { L"GM", L"Gambia" },
        { L"GE", L"Georgia" },
        { L"DE", L"Germany" },
        { L"GH", L"Ghana" },
        { L"GI", L"Gibraltar" },
        { L"GR", L"Greece" },
        { L"GL", L"Greenland" },
        { L"GD", L"Grenada" },
        { L"GP", L"Guadelupe" },
        { L"GU", L"Guam" },
        { L"GT", L"Guatemala" },
        { L"GG", L"Guernsey" },
        { L"GN", L"Guinea" },
        { L"GY", L"Guyana" },
        { L"HT", L"Haiti" },
        { L"VA", L"Holy See (Vatican City State)" },
        { L"HN", L"Honduras" },
        { L"HK", L"Hong Kong" },
        { L"HU", L"Hungary" },
        { L"IS", L"Iceland" },
        { L"IN", L"India" },
        { L"ID", L"Indonesia" },
        { L"IQ", L"Iraq" },
        { L"IE", L"Ireland" },
        { L"IR", L"Islamic Republic of Iran" },
        { L"IM", L"Isle of Man" },
        { L"IL", L"Israel" },
        { L"IT", L"Italy" },
        { L"JM", L"Jamaica" },
        { L"JP", L"Japan" },
        { L"JE", L"Jersey" },
        { L"JO", L"Jordan" },
        { L"KZ", L"Kazakhstan" },
        { L"KE", L"Kenya" },
        { L"KI", L"Kiribati" },
        { L"XK", L"Kosovo" },
        { L"KW", L"Kuwait" },
        { L"KG", L"Kyrgyzstan" },
        { L"LA", L"Lao People's Democratic Republic" },
        { L"LV", L"Latvia" },
        { L"LB", L"Lebanon" },
        { L"LS", L"Lesotho" },
        { L"LY", L"Libya" },
        { L"LI", L"Liechtenstein" },
        { L"LT", L"Lithuania" },
        { L"LU", L"Luxembourg" },
        { L"MO", L"Macao" },
        { L"MG", L"Madagascar" },
        { L"MW", L"Malawi" },
        { L"MY", L"Malaysia" },
        { L"MV", L"Maldives" },
        { L"ML", L"Mali" },
        { L"MT", L"Malta" },
        { L"MH", L"Marshall Islands" },
        { L"MQ", L"Martinique" },
        { L"MR", L"Mauritania" },
        { L"MU", L"Mauritius" },
        { L"YT", L"Mayotte" },
        { L"MX", L"Mexico" },
        { L"MD", L"Moldova" },
        { L"MC", L"Monaco" },
        { L"MN", L"Mongolia" },
        { L"ME", L"Montenegro" },
        { L"MS", L"Montserrat" },
        { L"MA", L"Morocco" },
        { L"MZ", L"Mozambique" },
        { L"MM", L"Myanmar" },
        { L"NA", L"Namibia" },
        { L"NP", L"Nepal" },
        { L"NL", L"Netherlands" },
        { L"NC", L"New Caledonia" },
        { L"NZ", L"New Zealand" },
        { L"NI", L"Nicaragua" },
        { L"NE", L"Niger" },
        { L"NG", L"Nigeria" },
        { L"NU", L"Niue" },
        { L"MK", L"North Macedonia" },
        { L"MP", L"Northern Mariana Islands" },
        { L"NO", L"Norway" },
        { L"OM", L"Oman" },
        { L"PK", L"Pakistan" },
        { L"PW", L"Palau" },
        { L"PA", L"Panama" },
        { L"PG", L"Papua New Guinea" },
        { L"PY", L"Paraguay" },
        { L"PE", L"Peru" },
        { L"PH", L"Philippines" },
        { L"PL", L"Poland" },
        { L"PT", L"Portugal" },
        { L"PR", L"Puerto Rico" },
        { L"QA", L"Qatar" },
        { L"RE", L"Reunion" },
        { L"RO", L"Romania" },
        { L"RU", L"Russian Federation" },
        { L"RW", L"Rwanda" },
        { L"BL", L"Saint Barthelemy" },
        { L"KN", L"Saint Kitts and Newis" },
        { L"LC", L"Saint Lucia" },
        { L"MF", L"Saint Martin" },
        { L"PM", L"Saint Pierre and Miquelon" },
        { L"VC", L"Saint Vincent and the Grenadines" },
        { L"WS", L"Samoa" },
        { L"SM", L"San Marino" },
        { L"ST", L"Sao Tome and Principe" },
        { L"SA", L"Saudi Arabia" },
        { L"SN", L"Senegal" },
        { L"RS", L"Serbia" },
        { L"SC", L"Seychelles" },
        { L"SL", L"Sierra Leone" },
        { L"SG", L"Singapore" },
        { L"SX", L"Sint Maarten" },
        { L"SK", L"Slovakia" },
        { L"SI", L"Slovenia" },
        { L"SB", L"Solomon Islands" },
        { L"SO", L"Somalia" },
        { L"ZA", L"South Africa" },
        { L"KR", L"South Korea" },
        { L"SS", L"South Sudan" },
        { L"ES", L"Spain" },
        { L"LK", L"Sri Lanka" },
        { L"PS", L"State of Palestine" },
        { L"SD", L"Sudan" },
        { L"SR", L"Suriname" },
        { L"SE", L"Sweden" },
        { L"CH", L"Switzerland" },
        { L"SY", L"Syrian Arab Republic" },
        { L"TW", L"Taiwan" },
        { L"TJ", L"Tajikistan" },
        { L"TH", L"Thailand" },
        { L"CD", L"The Democratic Republic of the Congo" },
        { L"TL", L"Timor-Leste" },
        { L"TG", L"Togo" },
        { L"TK", L"Tokelau" },
        { L"TO", L"Tonga" },
        { L"TT", L"Trinidad and Tobago" },
        { L"TN", L"Tunisia" },
        { L"TR", L"Türkiye" },
        { L"TM", L"Turkmenistan" },
        { L"TC", L"Turks and Caicos Islands" },
        { L"UG", L"Uganda" },
        { L"UA", L"Ukraine" },
        { L"AE", L"United Arab Emirates" },
        { L"GB", L"United Kingdom" },
        { L"TZ", L"United Republic of Tanzania" },
        { L"US", L"United States" },
        { L"UY", L"Uruguay" },
        { L"UZ", L"Uzbekistan" },
        { L"VU", L"Vanuatu" },
        { L"VE", L"Venezuela" },
        { L"VN", L"Vietnam" },
        { L"VG", L"Virgin Islands, British" },
        { L"VI", L"Virgin Islands, U.S." },
        { L"YE", L"Yemen" },
        { L"ZM", L"Zambia" },
        { L"ZW", L"Zimbabwe" }
    };
};