/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "x11_helper.h"

#define explicit explicit_is_keyword_in_cpp
#include <xcb/xkb.h>
#undef explicit


#include <QX11Info>
#include <QCoreApplication>
#include <QDebug>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <xcb/xcb.h>
#include <fixx11h.h>


// more information about the limit https://bugs.freedesktop.org/show_bug.cgi?id=19501
int X11Helper::MAX_GROUP_COUNT = 4;
int X11Helper::ARTIFICIAL_GROUP_LIMIT_COUNT = 8;

const char* X11Helper::LEFT_VARIANT_STR = "(";
const char* X11Helper::RIGHT_VARIANT_STR = ")";

bool X11Helper::xkbSupported(int* xkbOpcode)
{
    // Verify the Xlib has matching XKB extension.

    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;

    if (!XkbLibraryVersion(&major, &minor))
    {
        qWarning() << "Xlib XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    // Verify the X server has matching XKB extension.

    int opcode_rtrn;
    int error_rtrn;
    int xkb_opcode;
    if( ! XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn, &major, &minor)) {
        qWarning() << "X server XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion;
        return false;
    }

    if( xkbOpcode != NULL ) {
    	*xkbOpcode = xkb_opcode;
    }

    return true;
}

void X11Helper::switchToNextLayout()
{
	int size = getLayoutsList().size();	//TODO: could optimize a bit as we don't need the layouts - just count
	int group = (X11Helper::getGroup() + 1) % size;
	X11Helper::setGroup(group);
}

void X11Helper::scrollLayouts(int delta)
{
	int size = getLayoutsList().size();	//TODO: could optimize a bit as we don't need the layouts - just count
	int group = X11Helper::getGroup() + delta;
	group = group < 0 ? size - ((-group) % size) : group % size;

	X11Helper::setGroup(group);
}

QStringList X11Helper::getLayoutsListAsString(const QList<LayoutUnit>& layoutsList)
{
	QStringList stringList;
	foreach(const LayoutUnit& layoutUnit, layoutsList) {
		stringList << layoutUnit.toString();
	}
	return stringList;
}

bool X11Helper::setLayout(const LayoutUnit& layout)
{
	QList<LayoutUnit> currentLayouts = getLayoutsList();
	int idx = currentLayouts.indexOf(layout);
	if( idx == -1 || idx >= X11Helper::MAX_GROUP_COUNT ) {
		qWarning() << "Layout" << layout.toString() << "is not found in current layout list"
								<< getLayoutsListAsString(currentLayouts);
		return false;
	}

	return X11Helper::setGroup((unsigned int)idx);
}

bool X11Helper::setDefaultLayout() {
	return X11Helper::setGroup(0);
}

bool X11Helper::isDefaultLayout() {
	return X11Helper::getGroup() == 0;
}

LayoutUnit X11Helper::getCurrentLayout()
{
	QList<LayoutUnit> currentLayouts = getLayoutsList();
	unsigned int group = X11Helper::getGroup();
	if( group < (unsigned int)currentLayouts.size() )
		return currentLayouts[group];

	qWarning() << "Current group number" << group << "is outside of current layout list" <<
						getLayoutsListAsString(currentLayouts);
	return LayoutUnit();
}

LayoutSet X11Helper::getCurrentLayouts()
{
	LayoutSet layoutSet;

	QList<LayoutUnit> currentLayouts = getLayoutsList();
	layoutSet.layouts = currentLayouts;

	unsigned int group = X11Helper::getGroup();
	if( group < (unsigned int)currentLayouts.size() ) {
		layoutSet.currentLayout = currentLayouts[group];
	}
	else {
		qWarning() << "Current group number" << group << "is outside of current layout list" << getLayoutsListAsString(currentLayouts);
		layoutSet.currentLayout = LayoutUnit();
	}

	return layoutSet;
}


//static QString addNum(const QString& str, int n)
//{
//    QString format("%1%2");
//    if( str.length() >= 3 ) return format.arg(str.left(2)).arg(n);
//    return format.arg(str).arg(n);
//}

QList<LayoutUnit> X11Helper::getLayoutsList()
{
	XkbConfig xkbConfig;
	QList<LayoutUnit> layouts;
	if( X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::LAYOUTS_ONLY) ) {
		for(int i=0; i<xkbConfig.layouts.size(); i++) {
			QString layout(xkbConfig.layouts[i]);
			QString variant;
			if( i<xkbConfig.variants.size() && ! xkbConfig.variants[i].isEmpty() ) {
				variant = xkbConfig.variants[i];
			}
			layouts << LayoutUnit(layout, variant);
		}
		// if there are layouts with same map name add numbers to display name
//		for(int i=0; i<layouts.length(); i++) {
//			int n=1;
//			for(int j=i+1; j<layouts.length(); j++) {
//				if( layouts[i].layout == layouts[j].layout && layouts[i].getRawDisplayName().isEmpty() ) {
//					layouts[i].setDisplayName( addNum(layouts[i].layout, 1) );
//					layouts[j].setDisplayName( addNum(layouts[j].layout, ++n) );
//					qDebug() << "Adding" << 1 << "to" << layouts[i].toString();
//					qDebug() << "Adding" << n << "to" << layouts[j].toString();
//				}
//			}
//		}
	}
	else {
		qWarning() << "Failed to get layout groups from X server";
	}
	return layouts;
}

bool X11Helper::setGroup(unsigned int group)
{
    qDebug() << group;
    xcb_void_cookie_t cookie;
    cookie = xcb_xkb_latch_lock_state(QX11Info::connection(),
        XCB_XKB_ID_USE_CORE_KBD,
        0, 0,
        1,
        group,
        0, 0, 0
    );
    xcb_generic_error_t *error = nullptr;
    error = xcb_request_check(QX11Info::connection(), cookie);
    if (error) {
        qDebug() << "Couldn't change the group" << error->error_code;
        return false;
    }

    return true;
}

unsigned int X11Helper::getGroup()
{
	XkbStateRec xkbState;
	XkbGetState( QX11Info::display(), XkbUseCoreKbd, &xkbState );
	return xkbState.group;
}

bool X11Helper::getGroupNames(Display* display, XkbConfig* xkbConfig, FetchType fetchType)
{
	static const char* OPTIONS_SEPARATOR = ",";

	Atom real_prop_type;
	int fmt;
	unsigned long nitems, extra_bytes;
	char *prop_data = NULL;
	Status ret;

	Atom rules_atom = XInternAtom(display, _XKB_RF_NAMES_PROP_ATOM, False);

	/* no such atom! */
	if (rules_atom == None) {       /* property cannot exist */
		qWarning() << "Failed to fetch layouts from server:" << "could not find the atom" << _XKB_RF_NAMES_PROP_ATOM;
		return false;
	}

	ret = XGetWindowProperty(display,
			DefaultRootWindow(display),
			rules_atom, 0L, _XKB_RF_NAMES_PROP_MAXLEN,
			False, XA_STRING, &real_prop_type, &fmt,
			&nitems, &extra_bytes,
			(unsigned char **) (void *) &prop_data);

	/* property not found! */
	if (ret != Success) {
		qWarning() << "Failed to fetch layouts from server:" << "Could not get the property";
		return false;
	}

	/* has to be array of strings */
	if ((extra_bytes > 0) || (real_prop_type != XA_STRING) || (fmt != 8)) {
		if (prop_data)
			XFree(prop_data);
		qWarning() << "Failed to fetch layouts from server:" << "Wrong property format";
		return false;
	}

//	qDebug() << "prop_data:" << nitems << prop_data;
	QStringList names;
	for(char* p=prop_data; p-prop_data < (long)nitems && p != NULL; p += strlen(p)+1) {
		names.append( p );
//		kDebug() << " " << p;
	}

	if( names.count() < 4 ) { //{ rules, model, layouts, variants, options }
		XFree(prop_data);
		return false;
	}

	if( fetchType == ALL || fetchType == LAYOUTS_ONLY ) {
		QStringList layouts = names[2].split(OPTIONS_SEPARATOR);
		QStringList variants = names[3].split(OPTIONS_SEPARATOR);

		for(int ii=0; ii<layouts.count(); ii++) {
			xkbConfig->layouts << (layouts[ii] != NULL ? layouts[ii] : "");
			xkbConfig->variants << (ii < variants.count() && variants[ii] != NULL ? variants[ii] : "");
		}
		qDebug() << "Fetched layout groups from X server:"
				<< "\tlayouts:" << xkbConfig->layouts
				<< "\tvariants:" << xkbConfig->variants;
	}

	if( fetchType == ALL || fetchType == MODEL_ONLY ) {
		xkbConfig->keyboardModel = (names[1] != NULL ? names[1] : "");
		qDebug() << "Fetched keyboard model from X server:" << xkbConfig->keyboardModel;
	}

	if( fetchType == ALL ) {
		if( names.count() >= 5 ) {
			QString options = (names[4] != NULL ? names[4] : "");
			xkbConfig->options = options.split(OPTIONS_SEPARATOR);
			qDebug() << "Fetched xkbOptions from X server:" << options;
		}
	}

	XFree(prop_data);
	return true;
}

XEventNotifier::XEventNotifier():
		xkbOpcode(-1)
{
	if( QCoreApplication::instance() == NULL ) {
		qWarning() << "Layout Widget won't work properly without QCoreApplication instance";
	}
}

void XEventNotifier::start()
{
	qDebug() << "qCoreApp" << QCoreApplication::instance();
	if( QCoreApplication::instance() != NULL && X11Helper::xkbSupported(&xkbOpcode) ) {
		registerForXkbEvents(QX11Info::display());

		// start the event loop
		QCoreApplication::instance()->installNativeEventFilter(this);
	}
}

void XEventNotifier::stop()
{
	if( QCoreApplication::instance() != NULL ) {
		//TODO: unregister
	//    XEventNotifier::unregisterForXkbEvents(QX11Info::display());

		// stop the event loop
		QCoreApplication::instance()->removeNativeEventFilter(this);
	}
}


bool XEventNotifier::isXkbEvent(xcb_generic_event_t* event)
{
//	kDebug() << "event response type:" << (event->response_type & ~0x80) << xkbOpcode << ((event->response_type & ~0x80) == xkbOpcode + XkbEventCode);
    return (event->response_type & ~0x80) == xkbOpcode + XkbEventCode;
}

bool XEventNotifier::processOtherEvents(xcb_generic_event_t* /*event*/)
{
	return true;
}

bool XEventNotifier::processXkbEvents(xcb_generic_event_t* event)
{
	_xkb_event *xkbevt = reinterpret_cast<_xkb_event *>(event);
	if( XEventNotifier::isGroupSwitchEvent(xkbevt) ) {
//		kDebug() << "group switch event";
		emit(layoutChanged());
	}
	else if( XEventNotifier::isLayoutSwitchEvent(xkbevt) ) {
//		kDebug() << "layout switch event";
		emit(layoutMapChanged());
	}
	return true;
}

bool XEventNotifier::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
//	kDebug() << "event type:" << eventType;
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
        if( isXkbEvent(ev) ) {
    		processXkbEvents(ev);
    	}
    	else {
    		processOtherEvents(ev);
    	}
    }
    return false;
}

//bool XEventNotifier::x11Event(XEvent * event)
//{
//	//    qApp->x11ProcessEvent ( event );
//	if( isXkbEvent(event) ) {
//		processXkbEvents(event);
//	}
//	else {
//		processOtherEvents(event);
//	}
//	return QWidget::x11Event(event);
//}

bool XEventNotifier::isGroupSwitchEvent(_xkb_event* xkbEvent)
{
//    XkbEvent *xkbEvent = (XkbEvent*) event;
#define GROUP_CHANGE_MASK \
    ( XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask )

    return xkbEvent->any.xkbType == XkbStateNotify && (xkbEvent->state_notify.changed & GROUP_CHANGE_MASK);
}

bool XEventNotifier::isLayoutSwitchEvent(_xkb_event* xkbEvent)
{
//    XkbEvent *xkbEvent = (XkbEvent*) event;

    return //( (xkbEvent->any.xkb_type == XkbMapNotify) && (xkbEvent->map.changed & XkbKeySymsMask) ) ||
/*    	  || ( (xkbEvent->any.xkb_type == XkbNamesNotify) && (xkbEvent->names.changed & XkbGroupNamesMask) || )*/
    	   (xkbEvent->any.xkbType == XkbNewKeyboardNotify);
}

int XEventNotifier::registerForXkbEvents(Display* display)
{
    int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
    if( ! XkbSelectEvents(display, XkbUseCoreKbd, eventMask, eventMask) ) {
    	qWarning() << "Couldn't select desired XKB events";
    	return false;
    }
    return true;
}


static const char* LAYOUT_VARIANT_SEPARATOR_PREFIX = "(";
static const char* LAYOUT_VARIANT_SEPARATOR_SUFFIX = ")";

static QString& stripVariantName(QString& variant)
{
	if( variant.endsWith(LAYOUT_VARIANT_SEPARATOR_SUFFIX) ) {
		int suffixLen = strlen(LAYOUT_VARIANT_SEPARATOR_SUFFIX);
		return variant.remove(variant.length()-suffixLen, suffixLen);
	}
	return variant;
}

LayoutUnit::LayoutUnit(const QString& fullLayoutName)
{
	QStringList lv = fullLayoutName.split(LAYOUT_VARIANT_SEPARATOR_PREFIX);
	layout = lv[0];
	variant = lv.size() > 1 ? stripVariantName(lv[1]) : "";
}

QString LayoutUnit::toString() const
{
	if( variant.isEmpty() )
		return layout;

	return layout + LAYOUT_VARIANT_SEPARATOR_PREFIX+variant+LAYOUT_VARIANT_SEPARATOR_SUFFIX;
}

const int LayoutUnit::MAX_LABEL_LENGTH = 3;
