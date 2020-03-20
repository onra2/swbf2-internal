#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <d3dx9math.h>
#pragma comment(lib, "d3dx9.lib")
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>
#include "D2DOverlay.h"

class MenuItem {
public:
	std::wstring name;
	int minimum = 0;
	int maximum = 1;
	int* setting = nullptr;
	int increment = 1;
	MenuItem(std::wstring _name) {
		this->name = _name;
	}
	MenuItem(std::wstring _name, int* _setting) {
		this->name = _name;
		this->setting = _setting;
	}
	MenuItem(std::wstring _name, int* _setting, int _minimum, int _maximum) {
		this->name = _name;
		this->setting = _setting;
		this->minimum = _minimum;
		this->maximum = _maximum;
	}
	MenuItem(std::wstring _name, int* _setting, int _minimum, int _maximum, int _inc) {
		this->name = _name;
		this->setting = _setting;
		this->minimum = _minimum;
		this->maximum = _maximum;
		this->increment = _inc;
	}
};

class MenuTab {
public:
	std::wstring label;
	MenuTab();
	MenuTab(std::wstring _label) {
		label = _label;
	}
	std::vector<MenuItem> Items = std::vector<MenuItem>();
	void AddItem(MenuItem item) {
		this->Items.push_back(item);
	}
};

class Menu {
private:
	Direct2DOverlay* Overlay;
public:
	bool show = true;

	int positionx = 20;
	int positiony = 20;
	int maxwidth = 375;
	int maxheight = 500;
	D3DXCOLOR Background = D3DXCOLOR(0, .5, .1, 1);
	D3DXCOLOR Borders = D3DXCOLOR(0, .8, .7, 1);
	D3DXCOLOR Text_Titles = D3DXCOLOR(1, 1, 1, 1);
	D3DXCOLOR Text_Items = D3DXCOLOR(.9, .8, .9, 1);

	int SelectedMenu = 0;
	int SelectedItem = 0;

	int spacing = 20;
	std::wstring title;
	std::vector<MenuTab> Tabs = std::vector<MenuTab>();
	Menu() {}

	void Initialize(Direct2DOverlay* _ov, std::wstring _title) {
		this->Overlay = _ov;
		this->title = _title;
	}
	void AddTab(MenuTab tab) {
		Tabs.push_back(tab);
	}
	void Render() {

		int heightm = (this->Tabs[this->SelectedMenu].Items.size() + 8) * spacing;
		if (!this->show) heightm = this->spacing * 4;

		if (this->show) 
		{
			this->Overlay->DrawBox(this->positionx, this->positiony, this->maxwidth, heightm, 1, Background.r, Background.g, Background.b, Background.a, true);
			this->Overlay->DrawString(this->title, 20, this->positionx + 10, this->positiony + spacing - 10, this->Text_Titles.r, this->Text_Titles.g, this->Text_Titles.b);

			for (int tabindex = 0; tabindex < Tabs.size(); tabindex++) {
				int widthx = ((this->maxwidth - this->positionx) / Tabs.size());
				float thicc = 2;
				if (this->SelectedMenu == tabindex) {
					thicc = 5;
				}
				int y1 = this->positiony + this->spacing * 2;
				int y2 = this->positiony + this->spacing * 3;
				int x1 = (this->positionx + widthx * tabindex) + thicc;
				int x2 = (this->positionx + widthx * (tabindex + 1)) - thicc;

				this->Overlay->DrawLine(x1, y1, x2, y1, thicc, Borders.r, Borders.g, Borders.g);
				this->Overlay->DrawLine(x2, y1, x2, y2, thicc, Borders.r, Borders.g, Borders.g);
				this->Overlay->DrawLine(x2, y2, x1, y2, thicc, Borders.r, Borders.g, Borders.g);
				this->Overlay->DrawLine(x1, y2, x1, y1, thicc, Borders.r, Borders.g, Borders.g);

				this->Overlay->DrawString(Tabs[tabindex].label, 15, x1 + 5, y1, this->Text_Titles.r, this->Text_Titles.g, this->Text_Titles.b);
			}

			for (int itemindex = 0; itemindex < this->Tabs[this->SelectedMenu].Items.size(); itemindex++) {
				this->Overlay->DrawString(this->Tabs[this->SelectedMenu].Items[itemindex].name,
					20, this->positionx + 30,
					this->positiony + (this->spacing * (4 + itemindex)),
					this->Text_Items.r, this->Text_Items.g, this->Text_Items.b);

				if (this->Tabs[this->SelectedMenu].Items[itemindex].setting != nullptr) {
					this->Overlay->DrawString(std::to_wstring(*this->Tabs[this->SelectedMenu].Items[itemindex].setting),
						20, (this->positionx + this->maxwidth) - 100,
						this->positiony + (this->spacing * (4 + itemindex)),
						this->Text_Items.r, this->Text_Items.g, this->Text_Items.b);
				}
			}

			this->Overlay->DrawString(L"->", 20, this->positionx + 5, this->positiony + (this->SelectedItem * (spacing)+(this->spacing * 4)), 1, 1, 0);

			if (GetAsyncKeyState(VK_TAB)) {
				this->SelectedMenu += 1;
				this->SelectedItem = 0;
				if (this->SelectedMenu > this->Tabs.size() - 1) {
					this->SelectedMenu = 0;
				}
				Sleep(150);
			}

			if (GetAsyncKeyState(VK_DOWN)) {
				this->SelectedItem += 1;
				if (this->SelectedItem > this->Tabs[this->SelectedMenu].Items.size() - 1) {
					this->SelectedItem = 0;
				}
				Sleep(150);
			}

			if (GetAsyncKeyState(VK_UP)) {
				this->SelectedItem -= 1;
				if (this->SelectedItem < 0) {
					this->SelectedItem = this->Tabs[this->SelectedMenu].Items.size() - 1;
				}
				Sleep(150);
			}

			if (GetAsyncKeyState(VK_RIGHT)) {
				if (this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting != nullptr) {
					*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting += this->Tabs[this->SelectedMenu].Items[this->SelectedItem].increment;
					if (*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting >
						this->Tabs[this->SelectedMenu].Items[this->SelectedItem].maximum) {
						*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting =
							this->Tabs[this->SelectedMenu].Items[this->SelectedItem].maximum;
					}
				}
				Sleep(150);
			}

			if (GetAsyncKeyState(VK_LEFT)) {
				if (this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting != nullptr) {
					*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting -= this->Tabs[this->SelectedMenu].Items[this->SelectedItem].increment;
					if (*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting <
						this->Tabs[this->SelectedMenu].Items[this->SelectedItem].minimum) {
						*this->Tabs[this->SelectedMenu].Items[this->SelectedItem].setting =
							this->Tabs[this->SelectedMenu].Items[this->SelectedItem].minimum;
					}
				}
				Sleep(150);
			}
		}

		if (GetAsyncKeyState(VK_DELETE)) {
			this->show = !this->show;
			Sleep(150);
		}
	}
};