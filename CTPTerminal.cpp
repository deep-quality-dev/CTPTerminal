// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QuoteService.h"
#include <set>
#include <iostream>

int main()
{
	CQuoteService quote_service;
	quote_service.LoadServerConfig();

	std::set<std::string> subscribe_products;
	subscribe_products.insert("rb");
	subscribe_products.insert("ru");
	subscribe_products.insert("au");
	subscribe_products.insert("ag");
	quote_service.SetSubscribeProducts(subscribe_products);

	quote_service.Initialize();

	char c;
	std::cin >> c;

    return 0;
}

