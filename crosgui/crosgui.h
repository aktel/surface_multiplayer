#ifndef CROSGUI_IBASEUI_H
#define CROSGUI_IBASEUI_H

#include <platform.h>

#ifdef PRAGMA_ONCE
#pragma once
#endif

abstract_class ICrosGui
{
public:
	void 
}

abstract_class IBaseUI
{
public:
	virtual void ShowQT( );
	virtual void ShowGame( );	// Auto spawn
	virtual void ShowGameConstruct( void *scheme ); // Read scheme and configuration elements from scheme  
}

#endif //CROSGUI_IBASEUI_H