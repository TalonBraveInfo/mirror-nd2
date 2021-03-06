import wx
import wx.xrc as xrc
import types

dlg_res = None
xrcResFileName = "buildsys3/gui/xrc/format_dlg.xrc"

def xrcSetFile(file):
    global xrcResFileName, dlg_res
    xrcResFileName = file
    dlg_res = None

def xrcCTRLUnpack(self, name, evt_pack=None):
    if name and name != "":
        id = xrc.XRCID(name)
        if evt_pack and isinstance(evt_pack, types.DictionaryType):
            for evt, evt_func in evt_pack.iteritems():
                if evt and evt_func:
                    evt(self, id, evt_func)
        return xrc.XRCCTRL(self.xrcRootItem, name)
    return None

def xrcCTRLUnknown(self, name, wxCtrlClass, evt_pack=None):
    if name and name != "":
        idXrc = xrc.XRCID(name)
        ctrl = wxCtrlClass(self)
        if ctrl is not None:
            dlg_res.AttachUnknownControl(name, ctrl, self)

            if evt_pack and isinstance(evt_pack, types.DictionaryType):
                for evt, evt_func in evt_pack.iteritems():
                    if evt and evt_func: evt(self, idXrc, evt_func)

            sizer = self.GetSizer()
            if sizer:
                sizer.SetVirtualSizeHints(self)
                self.SetAutoLayout(True)

                size = ctrl.GetSize()

                sizeBest = ctrl.GetBestSize()
                sizeBest.width = size.width

                container = ctrl.GetParent()
                sizer.SetItemMinSize(container, sizeBest.width, sizeBest.height)

                sizeMin = sizer.CalcMin()
                self.SetSizeHints(sizeMin.width, sizeMin.height)
                sizer.Fit(self)

            print "test"
            return ctrl
    return None

def xrcLoadPanel(self, parent, name):
    global dlg_res
    self.xrcRootItem = wx.PrePanel()
    if not dlg_res: dlg_res = xrc.XmlResource(xrcResFileName)
    dlg_res.LoadOnPanel(self.xrcRootItem, parent, name)
    self.PostCreate(self.xrcRootItem)

def xrcLoadDialog(self, parent, name):
    global dlg_res
    self.xrcRootItem = wx.PreDialog()
    if not dlg_res: dlg_res = xrc.XmlResource(xrcResFileName)
    dlg_res.LoadOnDialog(self.xrcRootItem, parent, name)
    self.PostCreate(self.xrcRootItem)

def xrcLoadFrame(self, parent, name):
    global dlg_res
    self.xrcRootItem = wx.PreFrame()
    if not dlg_res: dlg_res = xrc.XmlResource(xrcResFileName)
    dlg_res.LoadOnFrame(self.xrcRootItem, parent, name)
    self.PostCreate(self.xrcRootItem)
