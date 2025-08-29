
#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/textfile.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/timer.h>
#include <wx/dcbuffer.h>
#include <wx/colordlg.h>
#include <wx/toolbar.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>
#include <wx/config.h>
#include <wx/dir.h>

// ---------------- Settings ----------------
struct AppSettings {
    int width = 50;
    int height = 30;
    bool showGrid = true;
    bool showHUD = true;
    wxColour gridColor = *wxLIGHT_GREY;
    wxColour bgColor = *wxBLACK;
    wxColour aliveColor = *wxWHITE;
    wxString boundary = "Toroidal"; // "Finite" or "Toroidal"

    wxString SettingsPath() const {
        wxString dir = wxStandardPaths::Get().GetUserLocalDataDir();
        if (!wxDirExists(dir)) wxMkdir(dir);
        return wxFileName(dir, "settings.txt").GetFullPath();
    }

    void Reset() {
        *this = AppSettings(); // back to defaults
        Save();
    }

    void Save() const {
        wxTextFile file;
        wxString path = SettingsPath();
        if (wxFileExists(path)) wxRemoveFile(path);
        file.Create(path);
        file.AddLine(wxString::Format("width=%d", width));
        file.AddLine(wxString::Format("height=%d", height));
        file.AddLine(wxString::Format("showGrid=%d", showGrid));
        file.AddLine(wxString::Format("showHUD=%d", showHUD));
        file.AddLine(wxString::Format("gridColor=%d,%d,%d", gridColor.Red(), gridColor.Green(), gridColor.Blue()));
        file.AddLine(wxString::Format("bgColor=%d,%d,%d", bgColor.Red(), bgColor.Green(), bgColor.Blue()));
        file.AddLine(wxString::Format("aliveColor=%d,%d,%d", aliveColor.Red(), aliveColor.Green(), aliveColor.Blue()));
        file.AddLine(wxString::Format("boundary=%s", boundary));
        file.Write();
        file.Close();
    }

    void Load() {
        wxString path = SettingsPath();
        if (!wxFileExists(path)) { Save(); return; }
        wxTextFile file(path);
        if (!file.Open()) return;
        for ( size_t i=0; i<file.GetLineCount(); ++i ) {
            wxString line = file.GetLine(i);
            if (line.StartsWith("width=")) line.Mid(6).ToLong((long*)&width);
            else if (line.StartsWith("height=")) line.Mid(7).ToLong((long*)&height);
            else if (line.StartsWith("showGrid=")) { long v; line.Mid(9).ToLong(&v); showGrid = v!=0; }
            else if (line.StartsWith("showHUD=")) { long v; line.Mid(8).ToLong(&v); showHUD = v!=0; }
            else if (line.StartsWith("gridColor=")) {
                int r,g,b; sscanf(line.Mid(10).c_str(), "%d,%d,%d", &r,&g,&b); gridColor.Set(r,g,b);
            } else if (line.StartsWith("bgColor=")) {
                int r,g,b; sscanf(line.Mid(8).c_str(), "%d,%d,%d", &r,&g,&b); bgColor.Set(r,g,b);
            } else if (line.StartsWith("aliveColor=")) {
                int r,g,b; sscanf(line.Mid(11).c_str(), "%d,%d,%d", &r,&g,&b); aliveColor.Set(r,g,b);
            } else if (line.StartsWith("boundary=")) {
                boundary = line.Mid(9);
                if (!(boundary == "Finite" || boundary == "Toroidal")) boundary = "Toroidal";
            }
        }
        file.Close();
    }
};

// Forward declarations
class LifePanel;
class MainFrame;

// ---------------- Application ----------------
class LifeApp : public wxApp {
public:
    AppSettings settings;
    bool OnInit() override;
};

wxIMPLEMENT_APP(LifeApp);

// ---------------- Life Panel ----------------
class LifePanel : public wxPanel {
public:
    LifePanel(MainFrame* parent);

    void ResizeUniverse(int w, int h);
    void ClearUniverse();
    void Randomize(int percent=30);
    void NextGeneration();
    void ToggleCellAt(const wxPoint& pt);
    int AliveCount() const;
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    void SetColors(const wxColour& grid, const wxColour& bg, const wxColour& alive) {
        m_gridColor=grid; m_bgColor=bg; m_aliveColor=alive; Refresh();
    }
    void SetShowGrid(bool v){ m_showGrid=v; Refresh(); }
    void SetShowHUD(bool v){ m_showHUD=v; Refresh(); }
    bool GetShowGrid() const { return m_showGrid; }
    bool GetShowHUD() const { return m_showHUD; }
    void SetBoundary(const wxString& b){ m_boundary=b; }

    void SetGeneration(long g){ m_generation=g; Refresh(); }
    long GetGeneration() const { return m_generation; }

    // Serialization
    bool SaveUniverse(const wxString& path);
    bool LoadUniverse(const wxString& path, bool resizeToFile=true);

private:
    void OnPaint(wxPaintEvent&);
    void OnLeftDown(wxMouseEvent&);
    void OnSize(wxSizeEvent&);

    int m_width;
    int m_height;
    std::vector<bool> m_current;
    std::vector<bool> m_next;

    wxColour m_gridColor, m_bgColor, m_aliveColor;
    bool m_showGrid=true;
    bool m_showHUD=true;
    long m_generation=0;
    wxString m_boundary = "Toroidal";

    wxDECLARE_EVENT_TABLE();
};

enum {
    ID_TIMER = wxID_HIGHEST + 1,
    ID_START,
    ID_PAUSE,
    ID_NEXT,
    ID_RANDOMIZE,
    ID_NEW,
    ID_SAVE,
    ID_SAVEAS,
    ID_OPEN,
    ID_VIEW_GRID,
    ID_VIEW_HUD,
    ID_OPTIONS_COLORS,
    ID_OPTIONS_SIZE,
    ID_OPTIONS_BOUNDARY_FINITE,
    ID_OPTIONS_BOUNDARY_TOROIDAL,
    ID_SETTINGS_RESET
};

// ---------------- Main Frame ----------------
class MainFrame : public wxFrame {
public:
    MainFrame();
    void UpdateStatus();

    // Accessors
    LifePanel* GetPanel() { return m_panel; }

private:
    void BuildMenu();
    void BuildToolbar();

    // Menu handlers
    void OnQuit(wxCommandEvent&);
    void OnNew(wxCommandEvent&);
    void OnOpen(wxCommandEvent&);
    void OnSave(wxCommandEvent&);
    void OnSaveAs(wxCommandEvent&);
    void OnStart(wxCommandEvent&);
    void OnPause(wxCommandEvent&);
    void OnNext(wxCommandEvent&);
    void OnRandomize(wxCommandEvent&);
    void OnToggleGrid(wxCommandEvent&);
    void OnToggleHUD(wxCommandEvent&);
    void OnChooseColors(wxCommandEvent&);
    void OnChooseSize(wxCommandEvent&);
    void OnBoundaryFinite(wxCommandEvent&);
    void OnBoundaryToroidal(wxCommandEvent&);
    void OnResetSettings(wxCommandEvent&);
    void OnTimer(wxTimerEvent&);

    LifePanel* m_panel;
    wxTimer m_timer;
    bool m_running=false;
    int m_intervalMs=100; // speed
    wxString m_currentPath;

    wxDECLARE_EVENT_TABLE();
};

// ---------------- Event Tables ----------------
wxBEGIN_EVENT_TABLE(LifePanel, wxPanel)
    EVT_PAINT(LifePanel::OnPaint)
    EVT_LEFT_DOWN(LifePanel::OnLeftDown)
    EVT_SIZE(LifePanel::OnSize)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(ID_NEW, MainFrame::OnNew)
    EVT_MENU(ID_OPEN, MainFrame::OnOpen)
    EVT_MENU(ID_SAVE, MainFrame::OnSave)
    EVT_MENU(ID_SAVEAS, MainFrame::OnSaveAs)
    EVT_MENU(ID_START, MainFrame::OnStart)
    EVT_MENU(ID_PAUSE, MainFrame::OnPause)
    EVT_MENU(ID_NEXT, MainFrame::OnNext)
    EVT_MENU(ID_RANDOMIZE, MainFrame::OnRandomize)
    EVT_MENU(ID_VIEW_GRID, MainFrame::OnToggleGrid)
    EVT_MENU(ID_VIEW_HUD, MainFrame::OnToggleHUD)
    EVT_MENU(ID_OPTIONS_COLORS, MainFrame::OnChooseColors)
    EVT_MENU(ID_OPTIONS_SIZE, MainFrame::OnChooseSize)
    EVT_MENU(ID_OPTIONS_BOUNDARY_FINITE, MainFrame::OnBoundaryFinite)
    EVT_MENU(ID_OPTIONS_BOUNDARY_TOROIDAL, MainFrame::OnBoundaryToroidal)
    EVT_MENU(ID_SETTINGS_RESET, MainFrame::OnResetSettings)
    EVT_TIMER(ID_TIMER, MainFrame::OnTimer)
wxEND_EVENT_TABLE()

// ---------------- LifePanel Impl ----------------
LifePanel::LifePanel(MainFrame* parent)
: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxTAB_TRAVERSAL)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    auto& s = app->settings;
    m_width = s.width;
    m_height = s.height;
    m_showGrid = s.showGrid;
    m_showHUD = s.showHUD;
    m_gridColor = s.gridColor;
    m_bgColor = s.bgColor;
    m_aliveColor = s.aliveColor;
    m_boundary = s.boundary;

    m_current.assign(m_width*m_height, false);
    m_next.assign(m_width*m_height, false);
}

void LifePanel::ResizeUniverse(int w, int h) {
    m_width = w; m_height = h;
    m_current.assign(m_width*m_height, false);
    m_next.assign(m_width*m_height, false);
    m_generation = 0;
    Refresh();
}

void LifePanel::ClearUniverse() {
    std::fill(m_current.begin(), m_current.end(), false);
    m_generation = 0;
    Refresh();
}

void LifePanel::Randomize(int percent) {
    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x) {
            bool alive = (rand()%100) < percent;
            m_current[y*m_width + x] = alive;
        }
    }
    m_generation = 0;
    Refresh();
}

int LifePanel::AliveCount() const {
    int c=0; for (bool v : m_current) if (v) ++c; return c;
}

void LifePanel::ToggleCellAt(const wxPoint& pt) {
    // compute cell size
    wxSize sz = GetClientSize();
    int cellW = std::max(1, sz.GetWidth() / m_width);
    int cellH = std::max(1, sz.GetHeight() / m_height);
    int x = pt.x / cellW;
    int y = pt.y / cellH;
    if (x>=0 && x<m_width && y>=0 && y<m_height) {
        size_t idx = y*m_width + x;
        m_current[idx] = !m_current[idx];
        Refresh();
    }
}

static inline int mod(int a, int m) {
    int r = a % m; return (r<0)? r+m : r;
}

void LifePanel::NextGeneration() {
    auto aliveAt = [&](int x, int y)->bool{
        if (m_boundary == "Finite") {
            if (x<0 || x>=m_width || y<0 || y>=m_height) return false;
            return m_current[y*m_width+x];
        } else { // Toroidal
            int xx = mod(x, m_width);
            int yy = mod(y, m_height);
            return m_current[yy*m_width+xx];
        }
    };
    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x) {
            int n=0;
            for (int j=-1;j<=1;++j)
                for (int i=-1;i<=1;++i)
                    if (!(i==0 && j==0) && aliveAt(x+i, y+j)) ++n;
            bool curr = m_current[y*m_width+x];
            bool next = curr;
            if (curr && (n<2 || n>3)) next=false;
            else if (!curr && n==3) next=true;
            m_next[y*m_width+x] = next;
        }
    }
    m_current.swap(m_next);
    ++m_generation;
    Refresh();
}

bool LifePanel::SaveUniverse(const wxString& path) {
    wxTextFile file;
    if (wxFileExists(path)) wxRemoveFile(path);
    if (!file.Create(path)) return false;
    file.AddLine(wxString::Format("%d %d", m_width, m_height));
    for (int y=0;y<m_height;++y) {
        wxString row;
        row.reserve(m_width*2);
        for (int x=0;x<m_width;++x) {
            row += (m_current[y*m_width+x] ? '1' : '0');
        }
        file.AddLine(row);
    }
    file.Write(); file.Close();
    return true;
}

bool LifePanel::LoadUniverse(const wxString& path, bool resizeToFile) {
    wxTextFile file(path);
    if (!file.Open()) return false;
    if (file.GetLineCount()<2) { file.Close(); return false; }
    int w=0,h=0;
    {
        wxString header = file.GetLine(0);
        wxArrayString parts = wxSplit(header, ' ');
        if (parts.size()>=2) {
            parts[0].ToLong((long*)&w);
            parts[1].ToLong((long*)&h);
        } else {
            file.Close(); return false;
        }
    }
    if (resizeToFile) ResizeUniverse(w,h);
    int rows = std::min((int)file.GetLineCount()-1, m_height);
    for (int y=0; y<rows; ++y) {
        wxString row = file.GetLine(1+y);
        for (int x=0; x<std::min((int)row.length(), m_width); ++x) {
            m_current[y*m_width+x] = (row[x]=='1');
        }
    }
    m_generation=0;
    file.Close();
    Refresh();
    return true;
}

void LifePanel::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(m_bgColor));
    dc.Clear();

    wxSize sz = GetClientSize();
    int cellW = std::max(1, sz.GetWidth() / m_width);
    int cellH = std::max(1, sz.GetHeight() / m_height);

    // Draw alive cells
    dc.SetBrush(wxBrush(m_aliveColor));
    dc.SetPen(*wxTRANSPARENT_PEN);
    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x) {
            if (m_current[y*m_width+x]) {
                dc.DrawRectangle(x*cellW, y*cellH, cellW, cellH);
            }
        }
    }

    // Grid
    if (m_showGrid) {
        dc.SetPen(wxPen(m_gridColor));
        for (int x=0; x<=m_width; ++x) {
            dc.DrawLine(x*cellW, 0, x*cellW, m_height*cellH);
        }
        for (int y=0; y<=m_height; ++y) {
            dc.DrawLine(0, y*cellH, m_width*cellW, y*cellH);
        }
    }

    // HUD
    if (m_showHUD) {
        dc.SetTextForeground(*wxWHITE);
        wxString hud = wxString::Format("Gen: %ld  Alive: %d  Size: %dx%d  Boundary: %s",
            m_generation, AliveCount(), m_width, m_height, m_boundary);
        dc.DrawText(hud, 5, 5);
    }
}

void LifePanel::OnLeftDown(wxMouseEvent& e) {
    ToggleCellAt(e.GetPosition());
}

void LifePanel::OnSize(wxSizeEvent& e) {
    e.Skip();
    Refresh();
}

// ---------------- MainFrame Impl ----------------
MainFrame::MainFrame()
: wxFrame(nullptr, wxID_ANY, "Conway's Game of Life", wxDefaultPosition, wxSize(900,600)),
  m_timer(this, ID_TIMER)
{
    BuildMenu();
    BuildToolbar();
    CreateStatusBar(2);
    m_panel = new LifePanel(this);
    SetMinSize(wxSize(600,400));

    UpdateStatus();
}

void MainFrame::BuildMenu() {
    wxMenu* file = new wxMenu;
    file->Append(ID_NEW, "&New/Clear\tCtrl-N");
    file->Append(ID_OPEN, "&Open...\tCtrl-O");
    file->Append(ID_SAVE, "&Save\tCtrl-S");
    file->Append(ID_SAVEAS, "Save &As...");
    file->AppendSeparator();
    file->Append(wxID_EXIT, "E&xit");

    wxMenu* sim = new wxMenu;
    sim->Append(ID_START, "&Start\tF5");
    sim->Append(ID_PAUSE, "&Pause\tF6");
    sim->Append(ID_NEXT, "&Next\tSpace");
    sim->AppendSeparator();
    sim->Append(ID_RANDOMIZE, "&Randomize\tCtrl-R");

    wxMenu* view = new wxMenu;
    view->AppendCheckItem(ID_VIEW_GRID, "Show &Grid");
    view->AppendCheckItem(ID_VIEW_HUD, "Show &HUD");

    wxMenu* options = new wxMenu;
    options->Append(ID_OPTIONS_COLORS, "&Colors...");
    options->Append(ID_OPTIONS_SIZE, "&Universe Size...");
    wxMenu* boundary = new wxMenu;
    boundary->AppendRadioItem(ID_OPTIONS_BOUNDARY_FINITE, "&Finite");
    boundary->AppendRadioItem(ID_OPTIONS_BOUNDARY_TOROIDAL, "&Toroidal");
    options->AppendSubMenu(boundary, "&Boundary");

    wxMenu* settings = new wxMenu;
    settings->Append(ID_SETTINGS_RESET, "&Reset to Defaults");

    wxMenuBar* bar = new wxMenuBar;
    bar->Append(file, "&File");
    bar->Append(sim, "&Simulation");
    bar->Append(view, "&View");
    bar->Append(options, "&Options");
    bar->Append(settings, "&Settings");
    SetMenuBar(bar);

    // initialize checks based on current settings
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    GetMenuBar()->Check(ID_VIEW_GRID, app->settings.showGrid);
    GetMenuBar()->Check(ID_VIEW_HUD, app->settings.showHUD);
    if (app->settings.boundary == "Finite")
        GetMenuBar()->Check(ID_OPTIONS_BOUNDARY_FINITE, true);
    else
        GetMenuBar()->Check(ID_OPTIONS_BOUNDARY_TOROIDAL, true);
}

void MainFrame::BuildToolbar() {
    wxToolBar* tb = CreateToolBar(wxTB_HORIZONTAL | wxTB_TEXT);
    tb->AddTool(ID_START, "Start", wxArtProvider::GetBitmap(wxART_GO_FORWARD), "Start");
    tb->AddTool(ID_PAUSE, "Pause", wxArtProvider::GetBitmap(wxART_CROSS_MARK), "Pause");
    tb->AddTool(ID_NEXT, "Next", wxArtProvider::GetBitmap(wxART_GO_DIR_UP), "Next");
    tb->AddSeparator();
    tb->AddTool(ID_RANDOMIZE, "Randomize", wxArtProvider::GetBitmap(wxART_TIP), "Randomize");
    tb->Realize();
}

void MainFrame::OnQuit(wxCommandEvent&) {
    Close(true);
}

void MainFrame::OnNew(wxCommandEvent&) {
    m_panel->ClearUniverse();
    m_panel->SetGeneration(0);
    UpdateStatus();
}

void MainFrame::OnOpen(wxCommandEvent&) {
    wxFileDialog dlg(this, "Open Universe", "", "", "Universe files (*.txt)|*.txt|All files|*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal()==wxID_OK) {
        if (m_panel->LoadUniverse(dlg.GetPath(), true)) {
            m_currentPath = dlg.GetPath();
            UpdateStatus();
        } else {
            wxMessageBox("Failed to open file.", "Error", wxICON_ERROR|wxOK, this);
        }
    }
}

void MainFrame::OnSave(wxCommandEvent&) {
    if (m_currentPath.IsEmpty()) { OnSaveAs(*(new wxCommandEvent())); return; }
    if (!m_panel->SaveUniverse(m_currentPath))
        wxMessageBox("Failed to save file.", "Error", wxICON_ERROR|wxOK, this);
}

void MainFrame::OnSaveAs(wxCommandEvent&) {
    wxFileDialog dlg(this, "Save Universe As", "", "", "Universe files (*.txt)|*.txt|All files|*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal()==wxID_OK) {
        m_currentPath = dlg.GetPath();
        if (!m_panel->SaveUniverse(m_currentPath))
            wxMessageBox("Failed to save file.", "Error", wxICON_ERROR|wxOK, this);
    }
}

void MainFrame::OnStart(wxCommandEvent&) {
    if (!m_running) {
        m_timer.Start(m_intervalMs);
        m_running=true;
        UpdateStatus();
    }
}

void MainFrame::OnPause(wxCommandEvent&) {
    if (m_running) {
        m_timer.Stop();
        m_running=false;
        UpdateStatus();
    }
}

void MainFrame::OnNext(wxCommandEvent&) {
    if (!m_running) {
        m_panel->NextGeneration();
        UpdateStatus();
    }
}

void MainFrame::OnRandomize(wxCommandEvent&) {
    m_panel->Randomize(30);
    UpdateStatus();
}

void MainFrame::OnToggleGrid(wxCommandEvent& e) {
    bool show = e.IsChecked();
    m_panel->SetShowGrid(show);
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    app->settings.showGrid = show;
    app->settings.Save();
    UpdateStatus();
}

void MainFrame::OnToggleHUD(wxCommandEvent& e) {
    bool show = e.IsChecked();
    m_panel->SetShowHUD(show);
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    app->settings.showHUD = show;
    app->settings.Save();
    UpdateStatus();
}

void MainFrame::OnChooseColors(wxCommandEvent&) {
    // Grid
    {
        wxColourData data;
        data.SetChooseFull(true);
        data.SetColour(m_panel->GetForegroundColour());
        data.SetColour(m_panel->GetBackgroundColour());
        data.SetCustomColour(0, m_panel->GetForegroundColour());
        wxColourDialog dlg(this, &data);
        dlg.SetTitle("Choose Grid Color");
        if (dlg.ShowModal()==wxID_OK) {
            wxColour c = dlg.GetColourData().GetColour();
            LifeApp* app = (LifeApp*)wxApp::GetInstance();
            app->settings.gridColor = c;
            app->settings.Save();
            m_panel->SetColors(app->settings.gridColor, app->settings.bgColor, app->settings.aliveColor);
        }
    }
    // Background
    {
        wxColourData data;
        data.SetChooseFull(true);
        data.SetColour(*wxBLACK);
        wxColourDialog dlg(this, &data);
        dlg.SetTitle("Choose Background Color");
        if (dlg.ShowModal()==wxID_OK) {
            wxColour c = dlg.GetColourData().GetColour();
            LifeApp* app = (LifeApp*)wxApp::GetInstance();
            app->settings.bgColor = c;
            app->settings.Save();
            m_panel->SetColors(app->settings.gridColor, app->settings.bgColor, app->settings.aliveColor);
        }
    }
    // Alive
    {
        wxColourData data;
        data.SetChooseFull(true);
        data.SetColour(*wxWHITE);
        wxColourDialog dlg(this, &data);
        dlg.SetTitle("Choose Alive Cell Color");
        if (dlg.ShowModal()==wxID_OK) {
            wxColour c = dlg.GetColourData().GetColour();
            LifeApp* app = (LifeApp*)wxApp::GetInstance();
            app->settings.aliveColor = c;
            app->settings.Save();
            m_panel->SetColors(app->settings.gridColor, app->settings.bgColor, app->settings.aliveColor);
        }
    }
}

void MainFrame::OnChooseSize(wxCommandEvent&) {
    wxDialog dlg(this, wxID_ANY, "Universe Size", wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* topsz = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rowsz = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* wlbl = new wxStaticText(&dlg, wxID_ANY, "Width:");
    wxStaticText* hlbl = new wxStaticText(&dlg, wxID_ANY, "Height:");
    wxSpinCtrl* wspin = new wxSpinCtrl(&dlg, wxID_ANY);
    wxSpinCtrl* hspin = new wxSpinCtrl(&dlg, wxID_ANY);
    wspin->SetRange(5, 500); hspin->SetRange(5, 500);
    wspin->SetValue(m_panel->GetWidth());
    hspin->SetValue(m_panel->GetHeight());
    rowsz->Add(wlbl, 0, wxALIGN_CENTER|wxRIGHT, 8);
    rowsz->Add(wspin, 1, wxRIGHT, 20);
    rowsz->Add(hlbl, 0, wxALIGN_CENTER|wxRIGHT, 8);
    rowsz->Add(hspin, 1, 0, 0);
    topsz->Add(rowsz, 1, wxALL|wxEXPAND, 12);
    topsz->Add(dlg.CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND, 8);
    dlg.SetSizerAndFit(topsz);
    if (dlg.ShowModal()==wxID_OK) {
        int w = wspin->GetValue();
        int h = hspin->GetValue();
        m_panel->ResizeUniverse(w,h);
        LifeApp* app = (LifeApp*)wxApp::GetInstance();
        app->settings.width = w;
        app->settings.height = h;
        app->settings.Save();
        UpdateStatus();
    }
}

void MainFrame::OnBoundaryFinite(wxCommandEvent&) {
    m_panel->SetBoundary("Finite");
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    app->settings.boundary = "Finite";
    app->settings.Save();
    UpdateStatus();
}

void MainFrame::OnBoundaryToroidal(wxCommandEvent&) {
    m_panel->SetBoundary("Toroidal");
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    app->settings.boundary = "Toroidal";
    app->settings.Save();
    UpdateStatus();
}

void MainFrame::OnResetSettings(wxCommandEvent&) {
    LifeApp* app = (LifeApp*)wxApp::GetInstance();
    app->settings.Reset();
    // apply
    m_panel->ResizeUniverse(app->settings.width, app->settings.height);
    m_panel->SetColors(app->settings.gridColor, app->settings.bgColor, app->settings.aliveColor);
    m_panel->SetShowGrid(app->settings.showGrid);
    m_panel->SetShowHUD(app->settings.showHUD);
    m_panel->SetBoundary(app->settings.boundary);
    GetMenuBar()->Check(ID_VIEW_GRID, app->settings.showGrid);
    GetMenuBar()->Check(ID_VIEW_HUD, app->settings.showHUD);
    if (app->settings.boundary == "Finite")
        GetMenuBar()->Check(ID_OPTIONS_BOUNDARY_FINITE, true);
    else
        GetMenuBar()->Check(ID_OPTIONS_BOUNDARY_TOROIDAL, true);
    UpdateStatus();
}

void MainFrame::OnTimer(wxTimerEvent&) {
    m_panel->NextGeneration();
    UpdateStatus();
}

void MainFrame::UpdateStatus() {
    wxString state = m_running ? "Running" : "Paused";
    wxString left = wxString::Format("Gen: %ld  Alive: %d  Size: %dx%d",
        m_panel->GetGeneration(),
        m_panel->AliveCount(),
        m_panel->GetWidth(),
        m_panel->GetHeight());
    SetStatusText(left, 0);
    SetStatusText(state, 1);
}

// ---------------- LifeApp Impl ----------------
bool LifeApp::OnInit() {
    settings.Load();
    MainFrame* frame = new MainFrame();
    frame->Show(true);
    return true;
}
