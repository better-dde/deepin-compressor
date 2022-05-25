/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include <DWidget>
#include <DSpinner>
#include <DLabel>

DWIDGET_USE_NAMESPACE

// 加载界面
class LoadingPage : public DWidget
{
public:
    explicit LoadingPage(QWidget *parent = nullptr);
    ~LoadingPage() override;

    /**
     * @brief startLoading  开始加载
     */
    void startLoading();

    /**
     * @brief stopLoading   结束加载
     */
    void stopLoading();

    /**
     * @brief setDes    设置描述
     * @param strDes    描述内容
     */
    void setDes(const QString &strDes);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

private:
    DSpinner *m_pSpinner;   // 加载效果
    DLabel *m_pTextLbl;
};

#endif // LOADINGPAGE_H
