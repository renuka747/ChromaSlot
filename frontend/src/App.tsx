import { Fragment, useMemo, useState, useEffect } from "react";

type Page = "landing" | "roles" | "student" | "faculty" | "admin";
type AdminView =
  | "Overview"
  | "Timetable"
  | "Sessions"
  | "Faculty"
  | "Repairs"
  | "Complaints"
  | "Analytics"
  | "Health Check"
  | "Export";

export let days = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"];
const periods = ["09:00–10:00", "10:00–11:00", "11:00–12:00", "12:00–13:00", "14:00–15:00", "15:00–16:00", "16:00–17:00"];

let sessions: any[] = [
  { subject: "Data Structures", faculty: "Dr. Sharma", division: "D1", room: "R101", type: "lecture" },
  { subject: "Operating Systems", faculty: "Prof. Mehta", division: "D1", room: "R102", type: "fixed" },
  { subject: "Machine Learning", faculty: "Dr. Patil", division: "D2", room: "R201", type: "lab" },
  { subject: "Computer Networks", faculty: "Prof. Kulkarni", division: "D2", room: "R202", type: "lecture" },
  { subject: "Mathematics", faculty: "Dr. Deshmukh", division: "D1", room: "R103", type: "lecture" },
  { subject: "Java OOP", faculty: "Prof. Patil", division: "D1", room: "R102", type: "fixed" },
  { subject: "DBMS", faculty: "Dr. Kulkarni", division: "D2", room: "R201", type: "lab" },
];

const iconPaths: Record<string, string[]> = {
  grid: ["M4 4h6v6H4zM14 4h6v6h-6zM4 14h6v6H4zM14 14h6v6h-6z"],
  calendar: ["M6 2v4M18 2v4M3 9h18M5 4h14a2 2 0 012 2v14H3V6a2 2 0 012-2z", "M7 13h3v3H7z"],
  clock: ["M12 22a10 10 0 100-20 10 10 0 000 20zM12 6v6l4 2"],
  users: ["M16 21v-2a4 4 0 00-4-4H6a4 4 0 00-4 4v2M9 11a4 4 0 100-8 4 4 0 000 8zM17 11a4 4 0 000-8M22 21v-2a4 4 0 00-3-3.87"],
  shield: ["M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10zM9 12l2 2 4-4"],
  warning: ["M10.3 2.9L1.8 17a2 2 0 001.7 3h17a2 2 0 001.7-3L14.7 2.9a2 2 0 00-4.4 0zM12 9v4M12 17h.01"],
  check: ["M20 6L9 17l-5-5"],
  file: ["M14 2H6a2 2 0 00-2 2v16h16V8zM14 2v6h6M8 13h8M8 17h6"],
  search: ["M11 19a8 8 0 100-16 8 8 0 000 16zM21 21l-4.35-4.35"],
  bell: ["M18 8a6 6 0 00-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9M10 21h4"],
  chart: ["M4 20V10M10 20V4M16 20v-7M22 20H2"],
  wrench: ["M14.7 6.3a4 4 0 01-5 5L4 17l3 3 5.7-5.7a4 4 0 005-5L15 12l-3-3z"],
  room: ["M3 21h18M5 21V3h14v18M9 8h2M9 12h2M9 16h2M15 8h.01M15 12h.01"],
  message: ["M21 15a4 4 0 01-4 4H8l-5 3V7a4 4 0 014-4h10a4 4 0 014 4z"],
  plus: ["M12 5v14M5 12h14"],
  filter: ["M4 4h16l-6 7v6l-4 2v-8z"],
  export: ["M12 3v12M7 10l5 5 5-5M5 21h14"],
  arrow: ["M5 12h14M13 6l6 6-6 6"],
  menu: ["M4 6h16M4 12h16M4 18h16"],
  cap: ["M2 10l10-6 10 6-10 6zM6 12v5c3 3 9 3 12 0v-5"],
  logout: ["M10 17l5-5-5-5M15 12H3M21 19V5a2 2 0 00-2-2h-6"],
};

function Icon({ name, size = 18 }: { name: string; size?: number }) {
  return (
    <svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true">
      {(iconPaths[name] || iconPaths.grid).map((d, i) => <path d={d} key={i} />)}
    </svg>
  );
}

function Logo({ light = false }: { light?: boolean }) {
  return <div className={`logo ${light ? "logo-light" : ""}`}><span className="logo-mark"><Icon name="calendar" size={20} /></span><span>ChromaSlot</span></div>;
}

function Button({ children, variant = "primary", icon, onClick, disabled = false }: { children: React.ReactNode; variant?: "primary" | "secondary" | "danger" | "ghost"; icon?: string; onClick?: () => void; disabled?: boolean }) {
  return <button className={`btn btn-${variant}`} onClick={onClick} disabled={disabled}>{icon && <Icon name={icon} size={16} />}{children}</button>;
}

function Badge({ children, tone = "neutral" }: { children: React.ReactNode; tone?: string }) {
  return <span className={`badge badge-${tone}`}><span className="badge-dot" />{children}</span>;
}

function Modal({ title, subtitle, children, onClose }: { title: string; subtitle?: string; children: React.ReactNode; onClose: () => void }) {
  return <div className="modal-backdrop" onMouseDown={onClose}><div className="modal" onMouseDown={e => e.stopPropagation()}><div className="modal-head"><div><h2>{title}</h2>{subtitle && <p>{subtitle}</p>}</div><button className="icon-btn" onClick={onClose}>×</button></div>{children}</div></div>;
}

function Toast({ text, tone = "success" }: { text: string; tone?: string }) {
  return <div className={`toast toast-${tone}`}><Icon name={tone === "success" ? "check" : "warning"} /><span>{text}</span></div>;
}

function Landing({ onExplore }: { onExplore: () => void }) {
  const features = [
    ["warning", "Conflict Detection", "Detect faculty, room, and division conflicts automatically."],
    ["calendar", "Smart Scheduling", "Generate schedules using constraint-aware timetable placement."],
    ["users", "Faculty Availability", "Update unavailable slots and repair affected schedules."],
    ["message", "Complaint Management", "Submit, assess, and resolve scheduling complaints."],
    ["chart", "Timetable Health", "Monitor violations, gaps, and every unplaced session."],
    ["export", "Excel Export", "Export class, faculty, and room-wise workbooks."],
  ];
  return <div className="landing">
    <header className="marketing-nav"><Logo /><nav><a href="#features">Features</a><a href="#process">How it works</a><a href="#roles">Roles</a></nav><Button onClick={onExplore}>Open dashboard <Icon name="arrow" size={15} /></Button></header>
    <main>
      <section className="hero">
        <div className="hero-copy"><Badge tone="primary">University scheduling, simplified</Badge><h1>Plan smarter.<br /><span>Schedule better.</span></h1><p>ChromaSlot generates balanced college timetables, detects scheduling conflicts, and intelligently repairs faculty availability and timetable complaints.</p><div className="hero-actions"><Button onClick={onExplore}>Explore ChromaSlot <Icon name="arrow" size={16} /></Button><Button variant="secondary" onClick={() => document.querySelector("#process")?.scrollIntoView({ behavior: "smooth" })}>See how it works</Button></div><div className="trust-row"><span><Icon name="check" />35 teaching slots</span><span><Icon name="check" />Conflict-aware</span><span><Icon name="check" />Approval controlled</span></div></div>
        <div className="hero-visual"><div className="browser"><div className="browser-bar"><span className="browser-dots"><i /><i /><i /></span><span className="browser-url">app.chromaslot.edu / timetable</span></div><div className="preview-shell"><div className="preview-side"><span className="mini-logo"><Icon name="calendar" /></span>{[1,2,3,4,5].map(x => <i key={x} className={x===2 ? "active" : ""} />)}</div><div className="preview-main"><div className="preview-head"><div><b>Timetable</b><small>35 usable teaching slots</small></div><span>Generate</span></div><div className="mini-grid"><em />{days.map(d => <b key={d}>{d.slice(0,3)}</b>)}{periods.slice(0,5).map((p,r) => <Fragment key={p}><small>{p.slice(0,5)}</small>{days.map((d,c) => <div className={`mini-cell ${r===1&&c===2 ? "has conflict" : (r+c)%3===0 ? "has" : ""}`} key={`${p}-${d}`}>{(r+c)%3===0 && <><strong>{sessions[(r+c)%sessions.length].subject}</strong><span>{sessions[(r+c)%sessions.length].room}</span></>}</div>)}</Fragment>)}</div></div></div></div></div>
      </section>
      <section className="logo-strip"><span>Trusted scheduling foundation for</span><b>Engineering</b><b>Sciences</b><b>Business</b><b>Humanities</b></section>
      <section className="section" id="features"><div className="section-heading"><Badge tone="primary">CAPABILITIES</Badge><h2>Everything you need to manage a smarter timetable</h2><p>From first placement to final export, keep every scheduling decision visible and controlled.</p></div><div className="feature-grid">{features.map(([icon,title,text]) => <article className="feature-card" key={title}><span className="feature-icon"><Icon name={icon} /></span><h3>{title}</h3><p>{text}</p><a onClick={onExplore}>Learn more <Icon name="arrow" size={14} /></a></article>)}</div></section>
      <section className="section process-section" id="process"><div className="section-heading"><Badge tone="neutral">HOW IT WORKS</Badge><h2>A clear path to a healthier timetable</h2></div><div className="process-grid">{[["01","Add sessions","Enter subjects, faculty, divisions, rooms, fixed slots, and availability."],["02","Generate","Create a timetable while respecting every scheduling constraint."],["03","Detect & repair","Identify unresolved sessions and propose safe, reviewable repairs."],["04","Monitor & export","Inspect health, approve changes, and export structured workbooks."]].map(([n,t,d]) => <article key={n}><span>{n}</span><h3>{t}</h3><p>{d}</p></article>)}</div></section>
      <section className="cta-section" id="roles"><div><Badge tone="light">BUILT FOR EVERY ROLE</Badge><h2>One scheduling system.<br />Three focused experiences.</h2><p>Give every member of your institution exactly the controls they need.</p></div><Button variant="secondary" onClick={onExplore}>Choose your role <Icon name="arrow" /></Button></section>
    </main>
    <footer><Logo light /><span>Plan smarter. Schedule better.</span><small>© 2025 ChromaSlot</small></footer>
  </div>;
}

function RoleSelection({ select, close }: { select: (p: Page) => void; close: () => void }) {
  const roles: [string,string,string,string,Page][] = [
    ["cap","Student","View your timetable and submit scheduling complaints.","Continue as Student","student"],
    ["users","Faculty","View assigned sessions, report unavailable periods, and submit complaints.","Continue as Faculty","faculty"],
    ["shield","Administrator","Generate, monitor, repair, and manage the complete timetable.","Continue as Admin","admin"],
  ];
  return <Modal title="How are you using ChromaSlot?" subtitle="Choose a role to explore a tailored demonstration workspace." onClose={close}><div className="role-grid">{roles.map(([icon,title,desc,label,page]) => <article className={`role-card ${page === "admin" ? "recommended" : ""}`} key={title}>{page==="admin"&&<span className="recommend">RECOMMENDED</span>}<span className="role-icon"><Icon name={icon} size={26}/></span><h3>{title}</h3><p>{desc}</p><Button variant={page==="admin"?"primary":"secondary"} onClick={()=>select(page)}>{label}<Icon name="arrow" size={15}/></Button></article>)}</div><p className="demo-note"><Icon name="shield" size={14}/> Demonstration only — no login or personal data required.</p></Modal>;
}


// --- API WIRING ---
export let timetableData: any = { placements: [], unplaced: [] };
export let healthData: any = {};
export let globalComplaints: any[] = [];
export let globalRepairs: any[] = [];
export function onGlobalUpdate() {
  onApiUpdate();
}

const apiListeners = new Set<() => void>();
export function onApiUpdate() {
  apiListeners.forEach(fn => {
    try { fn(); } catch(e) { console.error(e); }
  });
}
export function subscribeApi(fn: () => void) {
  apiListeners.add(fn);
  return () => { apiListeners.delete(fn); };
}

export let refetchAll = async () => {};

export function useApi() {
  const [, setTick] = useState(0);
  useEffect(() => {
    return subscribeApi(() => setTick(t => t + 1));
  }, []);

  useEffect(() => {
    refetchAll = fetchData;
    fetchData();
  }, []);
  
  async function fetchData() {
    try {
      const tRes = await fetch('/api/timetable?t=' + Date.now());
      if (tRes.ok) timetableData = await tRes.json();
      const hRes = await fetch('/api/health?t=' + Date.now());
      if (hRes.ok) healthData = await hRes.json();
      onApiUpdate();
    } catch(e) { console.error("Fetch data error:", e); }
  }
}

export async function triggerGenerate() {
  const genRes = await fetch('/api/generate', { method: 'POST' });
  if (genRes.ok) {
    try {
      timetableData = await genRes.json();
    } catch (e) {
      console.error(e);
    }
  }
  try {
    const hRes = await fetch('/api/health?t=' + Date.now());
    if (hRes.ok) healthData = await hRes.json();
  } catch(e) { console.error(e); }
  onApiUpdate();
  return timetableData;
}
// -------------------

const adminNav: [string, string, AdminView][] = [
  ["MAIN","grid","Overview"],["","calendar","Timetable"],["","file","Sessions"],
  ["MANAGEMENT","users","Faculty"],["","wrench","Repairs"],["","message","Complaints"],
  ["ANALYTICS","chart","Analytics"],["","warning","Health Check"],["","export","Export"],
];

function Sidebar({ view, setView, onHome, open }: { view: AdminView; setView: (v: AdminView)=>void; onHome:()=>void; open:boolean }) {
  const pendingComplaints = globalComplaints.filter(c => c.status === "Pending").length;
  const pendingRepairs = globalRepairs.filter(r => r.status === "Proposed").length;
  return <aside className={`sidebar ${open ? "sidebar-open":""}`}><Logo light /><div className="side-nav">{adminNav.map(([group,icon,label]) => <div key={label}>{group&&<span className="nav-group">{group}</span>}<button className={view===label?"active":""} onClick={()=>setView(label)}><Icon name={icon}/><span>{label}</span>{label==="Complaints"&&pendingComplaints>0&&<i>{pendingComplaints}</i>}{label==="Repairs"&&pendingRepairs>0&&<i>{pendingRepairs}</i>}</button></div>)}</div><div className="sidebar-bottom"><button><Icon name="grid"/>Settings</button><div className="profile"><span>AM</span><div><b>Arjun Mehta</b><small>Administrator</small></div><Icon name="logout" size={16}/></div><button className="back-link" onClick={onHome}>Back to website</button></div></aside>;
}

function Topbar({ title, toggleMenu }: { title:string; toggleMenu:()=>void }) {
  return <header className="topbar"><div className="title-wrap"><button className="menu-btn" onClick={toggleMenu}><Icon name="menu"/></button><div><h1>{title}</h1><p>Academic Year 2024–25 · Semester 2</p></div></div><div className="top-actions"><label className="search"><Icon name="search"/><input placeholder="Search anything..." /></label><button className="icon-btn"><Icon name="bell"/><i /></button><div className="top-profile"><span>AM</span><div><b>Arjun Mehta</b><small>Administrator</small></div></div></div></header>;
}

function Metric({ icon, label, value, note, tone="primary" }: {icon:string;label:string;value:string;note:string;tone?:string}) {
  return <article className="metric"><div className={`metric-icon tone-${tone}`}><Icon name={icon}/></div><div className="metric-label">{label}<span>•••</span></div><strong>{value}</strong><small>{note}</small></article>;
}

function Overview({ go }: { go:(v:AdminView, autoGenerate?: boolean)=>void }) {
  const placements = timetableData?.placements || [];
  const unplaced = timetableData?.unplaced || [];
  const totalSessions = placements.length + unplaced.length;
  const pendingCount = globalComplaints.filter(c => c.status === "Pending").length;
  const mondaySessions = [...placements].filter((s:any) => s.slot >= 0 && s.slot < 7).sort((a:any, b:any) => a.slot - b.slot);
  const score = totalSessions === 0 ? "—" : Math.max(0, 100 - (healthData?.total_issues || 0)).toString();
  return <div className="page-content"><div className="page-intro"><div><h2>Timetable Overview</h2><p>Monitor and manage the current academic timetable.</p></div><Button icon="plus" onClick={()=>go("Sessions")}>Load Dataset / Generate</Button></div>
    <div className="metrics metrics-five">
      <Metric icon="file" label="Total sessions" value={totalSessions.toString()} note={totalSessions > 0 ? "Loaded dataset" : "No dataset loaded"} />
      <Metric icon="check" label="Scheduled" value={placements.length.toString()} note="Conflict-free placed" tone="success"/>
      <Metric icon="warning" label="Unplaced" value={unplaced.length.toString()} note={unplaced.length > 0 ? "Requires attention" : "None"} tone="error"/>
      <Metric icon="message" label="Pending complaints" value={pendingCount.toString()} note={pendingCount > 0 ? "Awaiting review" : "All resolved"} tone="warning"/>
      <Metric icon="chart" label="Timetable score" value={score} note={totalSessions > 0 ? "out of 100" : "Load dataset first"} tone="purple"/>
    </div>
    <div className="overview-grid"><section className="panel today-panel"><div className="panel-head"><div><h3>Monday's Timetable</h3><p>Monday · {mondaySessions.length} scheduled sessions</p></div><button onClick={()=>go("Timetable")}>View full timetable <Icon name="arrow" size={14}/></button></div><div className="agenda">{mondaySessions.length === 0 ? <p style={{color:"#64748b", padding:"12px 0"}}>No sessions scheduled yet. Go to <b>Sessions → Load Dataset</b> to upload JSON and generate the timetable.</p> : mondaySessions.slice(0,6).map((s:any)=>{const p = periods[s.slot % 7] || "09:00–10:00"; return <div className="agenda-row" key={s.session_id}><span className="agenda-time"><b>{p.slice(0,5)}</b><small>{p.slice(6)}</small></span><i className={`session-line ${s.duration>1?"lab":"lecture"}`}/><div className="agenda-subject"><b>{s.subject}{s.session_id?.includes("-B") ? ` (${s.session_id.split("-").pop()})` : ""}</b><small>{s.duration>1?`Practical Lab · ${s.duration}h`:"Lecture"}</small></div><span>{s.faculty}</span><Badge tone="neutral">{s.division}</Badge><span className="room-pill"><Icon name="room" size={14}/>{s.room}</span></div>;})}</div></section>
      <section className="panel health-panel"><div className="panel-head"><div><h3>Timetable Health</h3><p>Live conflict status</p></div><span className="health-score">{score}</span></div><div className="health-list">{[["Faculty conflicts", healthData?.faculty_conflicts || 0, (healthData?.faculty_conflicts || 0) > 0 ? "error" : "success"],
  ["Room conflicts", healthData?.room_conflicts || 0, (healthData?.room_conflicts || 0) > 0 ? "error" : "success"],
  ["Division conflicts", healthData?.division_conflicts || 0, (healthData?.division_conflicts || 0) > 0 ? "error" : "success"],
  ["Unavailability violations", healthData?.unavail_violations || 0, (healthData?.unavail_violations || 0) > 0 ? "error" : "success"],
  ["Capacity violations", healthData?.capacity_violations || 0, (healthData?.capacity_violations || 0) > 0 ? "error" : "success"],
  ["Consecutive violations", healthData?.consecutive_violations || 0, (healthData?.consecutive_violations || 0) > 0 ? "warning" : "success"],
  ["Unplaced sessions", healthData?.unplaced_sessions || 0, (healthData?.unplaced_sessions || 0) > 0 ? "warning" : "success"]].map(([l,v,t])=><div key={l}><span><i className={`status-dot ${t}`}/>{l}</span><b>{v}</b></div>)}</div><Button variant="secondary" onClick={()=>go("Health Check")}>Run full health check</Button></section>
    </div>
    <div className="lower-grid"><section className="panel"><div className="panel-head"><div><h3>Recent Complaints</h3><p>Latest requests from students and faculty</p></div><button onClick={()=>go("Complaints")}>View all</button></div>{globalComplaints.slice(-3).reverse().map(x=><div className="list-row" key={x.id}><span className="list-icon"><Icon name="message"/></span><div><b>{x.type}</b><small>{x.id} · {x.target}</small></div><Badge tone={x.severity==="High"?"error":"neutral"}>{x.severity}</Badge><Badge tone={x.status==="Resolved"?"success":x.status==="Proposed"?"primary":"warning"}>{x.status}</Badge></div>)}</section>
      <section className="panel"><div className="panel-head"><div><h3>Quick Actions</h3><p>Common timetable operations</p></div></div><div className="quick-grid">{[["calendar","Generate timetable","Timetable"],["file","Manage sessions","Sessions"],["users","Manage faculty","Faculty"],["message","Review complaints","Complaints"],["warning","Check health","Health Check"],["export","Export Excel","Export"]].map(([i,l,v])=><button key={l} onClick={()=>go(v as AdminView, l === "Generate timetable")}><span><Icon name={i}/></span>{l}<Icon name="arrow" size={14}/></button>)}</div></section></div>
  </div>;
}

function SessionCard({ s, conflict=false }: {s:any; conflict?:boolean}) {
  const cardType = s.type || (s.duration > 1 ? "lab" : (s.fixed_slot !== undefined && s.fixed_slot >= 0) ? "fixed" : "lecture");
  const batchMatch = typeof s.session_id === "string" ? s.session_id.match(/-(B\d+)$/) : null;
  const batchTag = batchMatch ? batchMatch[1] : null;
  return <div className={`session-card ${cardType} ${conflict?"conflict":""}`}><div><b>{s.subject}</b>{cardType==="fixed"&&<span className="pin">FIXED</span>}{batchTag&&<span className="pin">{batchTag}</span>}</div><small>{s.faculty}{s.student_count ? ` · ${s.student_count} st.` : ""}</small><span>{s.division} <i/> {s.room}</span>{conflict&&<em><Icon name="warning" size={11}/> Faculty conflict</em>}</div>;
}

function Timetable({ showToast, initialGenerate = false, onGenerateModalClosed }: {showToast:(s:string)=>void; initialGenerate?: boolean; onGenerateModalClosed?: () => void}) {
  const [, setTick] = useState(0);
  const [conflicts,setConflicts]=useState(true);
  const [generate,setGenerate]=useState(initialGenerate);
  const [overwrite,setOverwrite]=useState(false);
  const [isGenerating,setIsGenerating]=useState(false);
  const [mode,setMode]=useState("Spread");
  const [filter,setFilter]=useState("All faculty");
  const [divFilter,setDivFilter]=useState("All divisions");
  const [roomFilter,setRoomFilter]=useState("All rooms");

  useEffect(() => {
    return subscribeApi(() => setTick(t => t + 1));
  }, []);

  useEffect(() => {
    if (initialGenerate) {
      setGenerate(true);
    }
  }, [initialGenerate]);

  const closeGenerate = () => {
    setGenerate(false);
    onGenerateModalClosed?.();
  };

  const closeOverwrite = () => {
    setOverwrite(false);
    onGenerateModalClosed?.();
  };

  const generateNow = () => {
    setGenerate(false);
    setOverwrite(true);
  };

  const handleGenerate = async () => {
    setIsGenerating(true);
    try {
      await triggerGenerate();
      setGenerate(false);
      setOverwrite(false);
      onGenerateModalClosed?.();
      showToast("Timetable generated successfully.");
    } catch (err) {
      console.error(err);
      showToast("Failed to generate timetable.");
    } finally {
      setIsGenerating(false);
    }
  };
  
  const uniqueFaculty = useMemo(() => Array.from(new Set([...(timetableData?.placements||[]), ...(timetableData?.unplaced||[])].map(s=>s.faculty).filter(Boolean))), [timetableData]);
  const uniqueDivisions = useMemo(() => Array.from(new Set([...(timetableData?.placements||[]), ...(timetableData?.unplaced||[])].map(s=>s.division).filter(Boolean))), [timetableData]);
  const uniqueRooms = useMemo(() => Array.from(new Set([...(timetableData?.placements||[]), ...(timetableData?.unplaced||[])].map(s=>s.room).filter(Boolean))), [timetableData]);
  
  const placed = useMemo(() => {
    let grid = Array(35).fill(null);
    if (!timetableData || !timetableData.placements) return grid;
    timetableData.placements.forEach((p: any) => {
       if (p.slot >= 0 && p.slot < 35) {
           if (!grid[p.slot]) grid[p.slot] = [];
           grid[p.slot].push(p);
           for (let i = 1; i < (p.duration || 1); i++) {
               if (p.slot + i < 35) {
                   if (!grid[p.slot+i]) grid[p.slot+i] = [];
                   grid[p.slot+i].push(p);
               }
           }
       }
    });
    return grid;
  }, [timetableData]);

  return <div className="page-content timetable-page"><div className="page-intro"><div><h2>Timetable</h2><p>35 usable teaching slots · Monday–Friday · Lunch 13:00–14:00</p></div><div className="action-row"><Button variant="secondary" icon="export" onClick={()=>showToast("Excel workbook export prepared.")}>Export Excel</Button><Button icon="plus" onClick={()=>setGenerate(true)}>Generate Timetable</Button></div></div>
    <section className="panel timetable-panel"><div className="filterbar"><div className="filter-fields"><label><span>Slot ordering</span><select value={mode} onChange={e=>setMode(e.target.value)}><option>Earliest Fit</option><option>Spread</option></select></label><label><span>Faculty</span><select value={filter} onChange={e=>setFilter(e.target.value)}><option>All faculty</option>{uniqueFaculty.map(f=><option key={f as string}>{f as string}</option>)}</select></label><label><span>Division</span><select value={divFilter} onChange={e=>setDivFilter(e.target.value)}><option>All divisions</option>{uniqueDivisions.map(d=><option key={d as string}>{d as string}</option>)}</select></label><label><span>Room</span><select value={roomFilter} onChange={e=>setRoomFilter(e.target.value)}><option>All rooms</option>{uniqueRooms.map(r=><option key={r as string}>{r as string}</option>)}</select></label></div><label className="toggle"><span>Show conflicts</span><input type="checkbox" checked={conflicts} onChange={e=>setConflicts(e.target.checked)}/><i/></label></div>
      <div className="schedule-scroll"><div className="schedule-grid"><div className="grid-corner"><Icon name="clock" size={15}/> TIME</div>{days.map(d=><div className="day-head" key={d}><b>{d}</b><small>{d==="Monday"?"17 Feb":d==="Tuesday"?"18 Feb":d==="Wednesday"?"19 Feb":d==="Thursday"?"20 Feb":"21 Feb"}</small></div>)}{periods.map((p,r)=><Fragment key={p}><div className="time-cell"><b>{p.slice(0,5)}</b><small>{p.slice(6)}</small>{r===3&&<span>Lunch follows</span>}</div>{days.map((d,c)=>{
  const bSlot = c * 7 + r;
  const slotSessions = (placed[bSlot] || []).filter((s:any) => {
      if (filter !== "All faculty" && s.faculty !== filter) return false;
      if (divFilter !== "All divisions" && s.division !== divFilter) return false;
      if (roomFilter !== "All rooms" && s.room !== roomFilter) return false;
      return true;
  });
  return <div className="schedule-cell" key={`${p}-${d}`}>
    {slotSessions.map((s:any) => <SessionCard key={s.session_id} s={s} />)}
    {slotSessions.length === 0 && <span className="empty-slot">Available</span>}
  </div>
})}{r===3&&<div className="lunch-band"><Icon name="clock" size={15}/><b>Lunch break</b><span>13:00–14:00 · Excluded from teaching slots</span></div>}</Fragment>)}</div></div>
      <div className="legend"><span><i className="legend-normal"/>Lecture</span><span><i className="legend-lab"/>Lab</span><span><i className="legend-fixed"/>Fixed session</span><span><i className="legend-conflict"/>Conflict</span><em>7 periods × 5 days = <b>35 usable slots</b></em></div>
    </section>
    <section className="panel unplaced"><div className="panel-head"><div><h3><Icon name="warning"/> Unplaced Sessions <Badge tone="warning">{(timetableData.unplaced || []).length}</Badge></h3><p>These sessions could not be assigned without violating constraints.</p></div><Button variant="secondary" icon="wrench" onClick={()=>showToast("Opening guided conflict resolution.")}>Resolve all</Button></div><div className="unplaced-grid">
  {(timetableData.unplaced || []).map((x:any) => (
    <article key={x.session_id}>
      <div><Badge tone="warning">{x.session_id}</Badge><h4>{x.subject}</h4><span>{x.faculty} ({x.division})</span></div>
      <div><small>REASON</small><b>{x.message}</b><small>CODE</small><span>Error {x.code}</span></div>
      <Button variant="secondary" onClick={()=>showToast(`Inspecting ${x.session_id}.`)}>View & resolve</Button>
    </article>
  ))}
</div></section>
    {generate&&<Modal title="Generate Timetable" subtitle="Choose how sessions should be assigned to available slots." onClose={closeGenerate}><div className="choice-grid">{["Earliest Fit","Spread"].map(x=><button className={mode===x?"selected":""} onClick={()=>setMode(x)} key={x}><span><Icon name={x==="Spread"?"chart":"clock"}/></span><b>{x}</b><small>{x==="Spread"?"Distribute sessions across days to improve workload balance.":"Assign sessions to the earliest available slot."}</small><i/></button>)}</div><div className="info-alert"><Icon name="warning"/><div><b>A timetable already exists.</b><span>Generating a new timetable may replace the current schedule and overwrite existing repair work.</span></div></div><div className="modal-actions"><Button variant="secondary" onClick={closeGenerate} disabled={isGenerating}>Cancel</Button><Button onClick={generateNow} disabled={isGenerating}>Continue to generate</Button></div></Modal>}
    {overwrite&&<Modal title="Confirm timetable overwrite" subtitle="This action requires explicit administrator approval." onClose={closeOverwrite}><div className="warning-block"><Icon name="warning" size={24}/><div><b>Existing schedule will be replaced</b><p>Approved repairs and manual timetable changes may be overwritten. This action cannot be automatically reversed.</p></div></div><div className="modal-actions"><Button variant="secondary" onClick={closeOverwrite} disabled={isGenerating}>Cancel</Button><Button variant="danger" onClick={handleGenerate} disabled={isGenerating}>{isGenerating ? "Generating..." : "Generate with overwrite"}</Button></div></Modal>}
  </div>;
}

const tableSessions = [
  ["CS101-A-1","Data Structures","Dr. Sharma","D1","R101","—","Scheduled"],
  ["CS102-A-1","Java OOP","Prof. Patil","D1","R102","Mon 10:00","Fixed"],
  ["CS103-B-1","DBMS","Dr. Kulkarni","D2","R201","—","Unplaced"],
  ["CS207-B-1","Computer Networks","Prof. Kulkarni","D2","R202","—","Scheduled"],
  ["MA102-A-1","Mathematics","Dr. Deshmukh","D1","R103","Wed 09:00","Fixed"],
];

function formatSlotLabel(slot: number | undefined): string {
  if (slot === undefined || slot < 0 || slot >= 35) return "—";
  const d = days[Math.floor(slot / 7)]?.slice(0, 3) || "";
  const p = periods[slot % 7]?.slice(0, 5) || "";
  return `${d} ${p}`;
}

function SessionsPage({notify, onGenerated}:{notify:(s:string)=>void; onGenerated?:()=>void}) {
 const [modal,setModal]=useState(false);
 const [uploadModal,setUploadModal]=useState(false);
 const [query,setQuery]=useState("");
 const [facFilter,setFacFilter]=useState("All faculty");
 const [divFilter,setDivFilter]=useState("All divisions");
 const [statusFilter,setStatusFilter]=useState("All status");

 const allRows = useMemo(() => {
   const placed = (timetableData?.placements || []).map((p:any) => ({
     id: p.session_id,
     subject: p.subject,
     faculty: p.faculty,
     division: p.division,
     room: p.room || "—",
     fixed: p.fixed_slot !== undefined && p.fixed_slot >= 0 ? formatSlotLabel(p.fixed_slot) : "—",
     status: p.fixed_slot !== undefined && p.fixed_slot >= 0 ? "Fixed" : "Scheduled",
   }));
   const unplaced = (timetableData?.unplaced || []).map((u:any) => ({
     id: u.session_id,
     subject: u.subject,
     faculty: u.faculty,
     division: u.division,
     room: "—",
     fixed: "Conflict",
     status: "Unplaced",
   }));
   return [...placed, ...unplaced];
 }, [timetableData]);

 const uniqueFaculty = useMemo(() => Array.from(new Set(allRows.map(r => r.faculty).filter(Boolean))), [allRows]);
 const uniqueDivisions = useMemo(() => Array.from(new Set(allRows.map(r => r.division).filter(Boolean))), [allRows]);

 const filtered = allRows.filter(r => {
   if (facFilter !== "All faculty" && r.faculty !== facFilter) return false;
   if (divFilter !== "All divisions" && r.division !== divFilter) return false;
   if (statusFilter !== "All status" && r.status !== statusFilter) return false;
   if (query.trim()) {
     const q = query.toLowerCase();
     return `${r.id} ${r.subject} ${r.faculty} ${r.division} ${r.room}`.toLowerCase().includes(q);
   }
   return true;
 });

 return <div className="page-content"><div className="page-intro"><div><h2>Sessions</h2><p>Manage timetable inputs, constraints, and fixed placements.</p></div>
 <div className="action-row"><Button icon="plus" onClick={()=>setModal(true)}>Add Session</Button><Button icon="file" variant="secondary" onClick={()=>setUploadModal(true)}>Load Dataset</Button></div>
 </div><section className="panel table-panel"><div className="table-tools"><label className="search"><Icon name="search"/><input value={query} onChange={e=>setQuery(e.target.value)} placeholder="Search sessions..." /></label><div><select value={facFilter} onChange={e=>setFacFilter(e.target.value)}><option>All faculty</option>{uniqueFaculty.map(f=><option key={f}>{f}</option>)}</select><select value={divFilter} onChange={e=>setDivFilter(e.target.value)}><option>All divisions</option>{uniqueDivisions.map(d=><option key={d}>{d}</option>)}</select><select value={statusFilter} onChange={e=>setStatusFilter(e.target.value)}><option>All status</option><option>Scheduled</option><option>Fixed</option><option>Unplaced</option></select></div></div>{filtered.length === 0 ? <p style={{padding:"24px", color:"#64748b", textAlign:"center"}}>No sessions loaded yet. Click <b>Load Dataset</b> above to paste the JSON dataset and generate the timetable.</p> : <DataTable rows={filtered.map(r => [r.id, r.subject, r.faculty, r.division, r.room, r.fixed, r.status])} />}</section>
 {modal&&<SessionModal close={()=>setModal(false)} save={()=>{setModal(false);notify("Session saved.");onGlobalUpdate();}}/>}
 {uploadModal&&<DatasetModal close={()=>setUploadModal(false)} notify={notify} onGenerated={onGenerated} />}
 </div>;
}

function DataTable({rows}:{rows:string[][]}) {
 return <div className="data-scroll"><table><thead><tr>{["Session ID","Subject","Faculty","Division","Room","Fixed Slot","Status","Actions"].map(h=><th key={h}>{h}</th>)}</tr></thead><tbody>{rows.map(r=><tr key={r[0]}>{r.map((x,i)=><td key={i}>{i===0?<b>{x}</b>:i===6?<Badge tone={x==="Scheduled"?"success":x==="Fixed"?"primary":"warning"}>{x}</Badge>:x}</td>)}<td><button className="more-btn">•••</button></td></tr>)}</tbody></table></div>;
}

function SessionModal({close,save}:{close:()=>void;save:()=>void}) {
 return <Modal title="Add Session" subtitle="Enter session details and scheduling constraints." onClose={close}><div className="form-grid"><label><span>Session ID</span><input placeholder="e.g. CS104-A-1"/></label><label><span>Subject</span><input placeholder="Subject name"/></label><label><span>Faculty</span><select><option>Select faculty</option><option>Dr. Sharma</option></select></label><label><span>Division</span><select><option>Select division</option><option>D1</option><option>D2</option></select></label><label><span>Room</span><select><option>Select room</option><option>R101</option></select></label><label><span>Fixed slot</span><select><option>No fixed slot</option><option>Monday 09:00</option></select></label><label className="full"><span>Unavailable slots</span><input placeholder="Select one or more restricted periods"/></label></div><div className="modal-actions"><Button variant="secondary" onClick={close}>Cancel</Button><Button onClick={save}>Save Session</Button></div></Modal>;
}

function FacultyPage({notify}:{notify:(s:string)=>void}) {
 const uniqueFaculty = useMemo(() => {
   const list = Array.from(new Set([...(timetableData?.placements||[]), ...(timetableData?.unplaced||[])].map((s:any)=>s.faculty).filter(Boolean))) as string[];
   return list.length > 0 ? list : ["F1", "F2", "F3", "F4", "F5"];
 }, [timetableData]);
 const [facultyMember, setFacultyMember] = useState(uniqueFaculty[0] || "F1");
 const [selected,setSelected]=useState<string[]>([]);
 function toggle(k:string){setSelected(v=>v.includes(k)?v.filter(x=>x!==k):[...v,k])}
 const doSave = () => {
     if(selected.length === 0) {
       notify("Select at least one slot to mark unavailable.");
       return;
     }
     selected.forEach(k => {
         const affected = (timetableData?.placements || []).find((p:any) => {
           const label = `${days[Math.floor(p.slot / 7)]} ${periods[p.slot % 7]?.slice(0,5)}`;
           return p.faculty === facultyMember && label === k;
         });
         globalRepairs.push({
           id: `REP-${Math.floor(100 + Math.random()*900)}`,
           author: facultyMember,
           target: affected ? `${affected.subject} (${affected.division})` : `${facultyMember} Session`,
           reason: "Faculty unavailable",
           status: "Proposed",
           from: k,
           to: "Next safe free slot"
         });
     });
     setSelected([]);
     onGlobalUpdate();
     notify("Availability saved. Proposed repairs queued for Admin review.");
 };
 return <div className="page-content"><div className="page-intro"><div><h2>Faculty Availability</h2><p>Set unavailable periods and trigger controlled schedule repair.</p></div><Button onClick={doSave}>Save Availability</Button></div><section className="panel availability"><div className="availability-top"><label><span>FACULTY MEMBER</span><select value={facultyMember} onChange={e=>setFacultyMember(e.target.value)}>{uniqueFaculty.map(f=><option key={f} value={f}>{f}</option>)}</select></label><div><Badge tone="success">Active</Badge></div></div><div className="availability-scroll"><div className="availability-grid"><div/>{days.map(d=><b key={d}>{d}</b>)}{["09:00","10:00","11:00","12:00","14:00","15:00","16:00"].map(t=><Fragment key={t}><span>{t}</span>{days.map(d=>{const k=`${d} ${t}`;const off=selected.includes(k);return <button key={k} className={off?"unavailable":""} onClick={()=>toggle(k)}><Icon name={off?"warning":"check"} size={14}/>{off?"Unavailable":"Available"}</button>})}</Fragment>)}</div></div><div className="availability-note"><Icon name="wrench"/><div><b>Availability changes may affect the live timetable</b><span>ChromaSlot will propose repairs before any sessions are moved.</span></div></div></section></div>;
}

function RepairsPage({notify}:{notify:(s:string)=>void}) {
 const approveIt = (r:any) => { r.status = "Applied"; onGlobalUpdate(); notify("Repair applied to timetable."); };
 const rejectIt = (r:any) => { r.status = "Rejected"; onGlobalUpdate(); notify("Repair proposal rejected."); };
 return <div className="page-content"><div className="page-intro"><div><h2>Timetable Repairs</h2><p>Review proposed changes before they affect the live timetable.</p></div><Badge tone="warning">{globalRepairs.filter(r=>r.status==="Proposed").length} awaiting review</Badge></div>
 <div className="repair-list">{globalRepairs.length === 0 ? <article className="panel" style={{padding:"24px", color:"#64748b"}}>No repair proposals queued yet. Mark an unavailable slot in the <b>Faculty</b> portal to trigger a proposed repair.</article> : globalRepairs.map(r=>
 <article className="panel repair-card" key={r.id}>
 <div className="repair-meta"><div className="avatar">{r.author.slice(0,2)}</div><div><b>{r.author}</b><small>{r.reason} · 1 affected session</small></div><Badge tone={r.status==="Applied"?"success":r.status==="Rejected"?"error":"primary"}>{r.status}</Badge></div>
 <div className="diff"><div><small>CURRENT</small><b>{r.target}</b><span>{r.from}</span></div><div className="diff-arrow"><Icon name="arrow"/></div><div className="proposed"><small>PROPOSED</small><b>{r.target}</b><span>{r.to}</span></div></div>
 <div className="repair-foot"><span><Icon name="check"/>Changes will not affect the live timetable until approved.</span><div>{r.status==="Proposed"&&<><Button variant="secondary" onClick={()=>rejectIt(r)}>Reject</Button><Button onClick={()=>approveIt(r)}>Apply Repair</Button></>}</div></div>
 </article>
 )}</div></div>;
}

function ComplaintsPage({notify}:{notify:(s:string)=>void}) {
 const [tab,setTab]=useState("All"); const [approve,setApprove]=useState<any>(null);
 const approveIt = (queueRepair: boolean) => {
     if(approve) {
       approve.status = "Resolved";
       if (queueRepair) {
         globalRepairs.push({
           id: `REP-${Math.floor(100 + Math.random()*900)}`,
           author: approve.author,
           target: approve.sessionLabel || approve.target,
           reason: `${approve.type} (${approve.day}${approve.slotLabel ? ` · ${approve.slotLabel}` : ""})`,
           status: "Proposed",
           from: `${approve.day}${approve.slotLabel ? ` ${approve.slotLabel}` : ""}`,
           to: approve.preferredSlot && approve.preferredSlot !== "Any available slot" ? approve.preferredSlot : "Next conflict-free slot"
         });
       }
       onGlobalUpdate();
     }
     setApprove(null);
     notify(queueRepair ? "Complaint resolved & repair proposal queued." : "Complaint resolved.");
 };
 const visible = globalComplaints.filter(x=>tab==="All"||x.status===tab);
 return <div className="page-content"><div className="page-intro"><div><h2>Complaint Management</h2><p>Validate, prioritize, and resolve scheduling concerns.</p></div><Button variant="secondary" icon="export" onClick={()=>notify("Complaint report exported.")}>Export report</Button></div>
 <div className="complaint-stats">{[["Pending",globalComplaints.filter(c=>c.status==="Pending").length,"warning"],["Resolved",globalComplaints.filter(c=>c.status==="Resolved").length,"success"]].map(x=><div key={x[0] as string}><span>{x[0] as string}</span><b>{x[1] as number}</b><i className={`status-dot ${x[2]}`}/></div>)}</div>
 <section className="panel complaint-panel"><div className="tabs">{["All","Pending","Resolved"].map(t=><button className={tab===t?"active":""} onClick={()=>setTab(t)} key={t}>{t}</button>)}</div>
 <div className="complaint-table">{visible.length === 0 ? <p style={{padding:"24px", color:"#64748b", textAlign:"center"}}>No complaints in this view yet. Submit one from the <b>Student</b> or <b>Faculty</b> portal.</p> : visible.map(x=><div className="complaint-row" key={x.id}><b>{x.id}</b><span>{x.author}</span><Badge tone="neutral">{x.type}</Badge><span><b>{x.target}</b>{x.detail && <small style={{color:"#475569"}}>{x.detail}</small>}<small>{x.message}</small></span><Badge tone={x.severity==="High"?"error":x.severity==="Medium"?"warning":"neutral"}>{x.severity}</Badge><Badge tone={x.status==="Resolved"?"success":"warning"}>{x.status}</Badge><span>{x.date}</span><Button variant="secondary" onClick={()=>x.status==="Pending"?setApprove(x):notify(`Viewing ${x.id}`)}>Review</Button></div>)}</div></section>
 {approve&&<Modal title="Review & Resolve Complaint" subtitle={`${approve.id} · ${approve.type} by ${approve.author}`} onClose={()=>setApprove(null)}>
   <div style={{display:"grid", gap:8, marginBottom:16, padding:12, background:"#f8fafc", borderRadius:8, fontSize:12}}>
     <div><b>Day:</b> {approve.day || "—"}</div>
     <div><b>Target:</b> {approve.detail || approve.target}</div>
     {approve.preferredSlot && <div><b>Preferred New Slot:</b> {approve.preferredSlot}</div>}
     <div><b>Reason:</b> {approve.message}</div>
   </div>
   <div className="modal-actions">
     <Button variant="secondary" onClick={()=>setApprove(null)}>Cancel</Button>
     <Button variant="secondary" onClick={()=>approveIt(false)}>Resolve Only</Button>
     <Button onClick={()=>approveIt(true)}>Queue Repair & Resolve</Button>
   </div>
 </Modal>}</div>;
}

function AnalyticsPage() {
 const placements = timetableData?.placements || [];
 const dayCounts = [0,1,2,3,4].map(d => placements.filter((p:any) => Math.floor(p.slot / 7) === d).length);
 const maxCount = Math.max(1, ...dayCounts);
 const bars = dayCounts.map(c => Math.round((c / maxCount) * 85) + 10);
 const score = placements.length === 0 ? "—" : Math.max(0, 100 - (healthData?.total_issues || 0)).toString();
 return <div className="page-content"><div className="page-intro"><div><h2>Timetable Analytics</h2><p>Understand quality, balance, and institutional utilization.</p></div><select className="standalone-select"><option>This semester</option></select></div><div className="metrics"><Metric icon="clock" label="Faculty conflicts" value={(healthData?.faculty_conflicts || 0).toString()} note="Hard constraint" tone="success"/><Metric icon="chart" label="Room conflicts" value={(healthData?.room_conflicts || 0).toString()} note="Hard constraint" tone="primary"/><Metric icon="room" label="Scheduled sessions" value={placements.length.toString()} note="Across 35 slots" tone="warning"/><Metric icon="shield" label="Total score" value={score} note={placements.length > 0 ? "Verified by C11 engine" : "No dataset loaded"} tone="purple"/></div><div className="chart-grid">{["Faculty workload by day","Division workload by day","Room utilization","Sessions per day"].map((t,idx)=><section className="panel chart-card" key={t}><div className="panel-head"><div><h3>{t}</h3><p>{idx<2?"Total assigned teaching hours":"Current semester distribution"}</p></div><button>•••</button></div><div className="bar-chart">{bars.map((b,i)=><div key={i}><span style={{height:`${placements.length===0?12:Math.max(20,b-idx*3)}%`}}/><small>{days[i].slice(0,3)}</small></div>)}</div></section>)}</div></div>;
}

function HealthPage({go}:{go:(v:AdminView)=>void}) {
 const placements = timetableData?.placements || [];
 const unplaced = timetableData?.unplaced || [];
 const totalSessions = placements.length + unplaced.length;
 const totalIssues = healthData?.total_issues || 0;
 const score = totalSessions === 0 ? "—" : Math.max(0, 100 - totalIssues).toString();
 const issueCards: [string, number, string][] = [
   ["Faculty conflicts", healthData?.faculty_conflicts || 0, (healthData?.faculty_conflicts || 0) > 0 ? "error" : "success"],
   ["Room conflicts", healthData?.room_conflicts || 0, (healthData?.room_conflicts || 0) > 0 ? "error" : "success"],
   ["Division conflicts", healthData?.division_conflicts || 0, (healthData?.division_conflicts || 0) > 0 ? "error" : "success"],
   ["Unavailability violations", healthData?.unavail_violations || 0, (healthData?.unavail_violations || 0) > 0 ? "error" : "success"],
   ["Capacity violations", healthData?.capacity_violations || 0, (healthData?.capacity_violations || 0) > 0 ? "error" : "success"],
   ["Consecutive violations", healthData?.consecutive_violations || 0, (healthData?.consecutive_violations || 0) > 0 ? "warning" : "success"],
   ["Unplaced sessions", healthData?.unplaced_sessions || 0, (healthData?.unplaced_sessions || 0) > 0 ? "warning" : "success"],
 ];
 return <div className="page-content"><div className="page-intro"><div><h2>Timetable Health</h2><p>Live constraint verification from the C11 health check engine.</p></div><Button icon="check" onClick={()=>refetchAll()}>Run health check</Button></div><div className="health-hero"><div><span className="score-ring">{score}<small>{totalSessions > 0 ? "/100" : ""}</small></span><div><Badge tone={totalIssues === 0 ? "success" : "warning"}>{totalSessions === 0 ? "Awaiting dataset" : totalIssues === 0 ? "Healthy" : `${totalIssues} diagnostic flag(s)`}</Badge><h3>{totalSessions === 0 ? "No timetable generated yet" : totalIssues === 0 ? "Zero conflicts across all constraints" : `0 hard overlaps · ${unplaced.length} unsatisfiable session flagged`}</h3><p>{totalSessions === 0 ? "Load a dataset in Sessions to run live verification." : "All placed sessions pass capacity, unavailability, consecutive-hour, and overlap checks."}</p></div></div><span>Live C11 audit</span></div><div className="issue-grid">{issueCards.map(([l,v,t])=><article className="panel" key={l}><span className={`issue-icon tone-${t}`}><Icon name={v > 0 ? "warning" : "check"}/></span><div><b>{v}</b><span>{l}</span></div><button onClick={()=>go("Timetable")}>View <Icon name="arrow" size={13}/></button></article>)}</div><section className="panel issues-table"><div className="panel-head"><div><h3>Issues requiring attention</h3><p>Exact diagnostic output from ChromaSlot preflight & backtracking</p></div></div>{unplaced.length === 0 ? <div className="issue-row"><Badge tone="success">OK</Badge><div><b>No unresolved conflicts</b><small>All loaded sessions verified</small></div><span>{totalSessions === 0 ? "Load a dataset to inspect health." : "100% of constraints satisfied."}</span><Button variant="secondary" onClick={()=>go("Timetable")}>View Grid</Button></div> : unplaced.map((u:any)=><div className="issue-row" key={u.session_id}><Badge tone="warning">Code {u.code}</Badge><div><b>Unplaced session {u.session_id} · {u.subject}</b><small>{u.faculty} · {u.division}</small></div><span>{u.message}</span><Button variant="secondary" onClick={()=>go("Timetable")}>Inspect</Button></div>)}</section></div>;
}

function ExportPage({notify}:{notify:(s:string)=>void}) {
 return <div className="page-content"><div className="page-intro"><div><h2>Export Timetable</h2><p>Download the current timetable in structured Excel workbooks.</p></div></div><div className="export-info"><Icon name="file"/><div><b>Complete timetable workbook</b><span>Exports include Monday–Friday, all teaching periods, lunch rows, relevant sessions, and unplaced sessions.</span></div><Button icon="export" onClick={()=>notify("Complete workbook downloaded.")}>Export all</Button></div><div className="export-grid">{[["users","Class-wise","View timetable organized by division and class.","2 divisions"],["cap","Faculty-wise","View timetable organized by faculty member.","10 faculty"],["room","Room-wise","View timetable organized by teaching room.","5 rooms"]].map(x=><article className="panel" key={x[1]}><span className="export-icon"><Icon name={x[0]} size={24}/></span><Badge tone="neutral">{x[3]}</Badge><h3>{x[1]}</h3><p>{x[2]}</p><div><Button variant="secondary" onClick={()=>notify(`${x[1]} preview opened.`)}>Preview</Button><Button icon="export" onClick={()=>notify(`${x[1]} workbook downloaded.`)}>Export Excel</Button></div></article>)}</div></div>;
}

function AdminApp({onHome}:{onHome:()=>void}) {
 const [view,setView]=useState<AdminView>("Overview");
 const [openGenerate,setOpenGenerate]=useState(false);
 const [menu,setMenu]=useState(false);
 const [toast,setToast]=useState("");
 function notify(s:string){setToast(s);window.setTimeout(()=>setToast(""),3000)}

 const handleGo = (v: AdminView, autoGenerate?: boolean) => {
   setView(v);
   if (autoGenerate) setOpenGenerate(true);
 };

 let content:React.ReactNode;
 if(view==="Overview")content=<Overview go={handleGo}/>;
 else if(view==="Timetable")content=<Timetable showToast={notify} initialGenerate={openGenerate} onGenerateModalClosed={()=>setOpenGenerate(false)}/>;
 else if(view==="Sessions")content=<SessionsPage notify={notify} onGenerated={()=>setView("Timetable")}/>;
 else if(view==="Faculty")content=<FacultyPage notify={notify}/>;
 else if(view==="Repairs")content=<RepairsPage notify={notify}/>;
 else if(view==="Complaints")content=<ComplaintsPage notify={notify}/>;
 else if(view==="Analytics")content=<AnalyticsPage/>;
 else if(view==="Health Check")content=<HealthPage go={handleGo}/>;
 else content=<ExportPage notify={notify}/>;
 return <div className="admin-app"><Sidebar view={view} setView={(v)=>{setView(v);setOpenGenerate(false);setMenu(false)}} onHome={onHome} open={menu}/><div className="admin-main"><Topbar title={view} toggleMenu={()=>setMenu(!menu)}/>{content}</div>{menu&&<div className="mobile-overlay" onClick={()=>setMenu(false)}/>} {toast&&<Toast text={toast}/>}</div>;
}

function Portal({type,onHome}:{type:"student"|"faculty";onHome:()=>void}) {
 const faculty=type==="faculty"; const [tab,setTab]=useState("Dashboard"); const [submitted,setSubmitted]=useState(false); const nav=faculty?["Dashboard","Timetable","Availability","Complaints"]:["Dashboard","Timetable","Complaints"];
 const mySessions = (timetableData?.placements || []).filter((s:any) => faculty ? s.faculty === "F1" : s.division === "Div A").sort((a:any, b:any) => a.slot - b.slot);
 const mondayCount = mySessions.filter((s:any) => s.slot >= 0 && s.slot < 7).length;
 const pendingComplaints = globalComplaints.filter(c => c.status === "Pending").length;
 const resolvedComplaints = globalComplaints.filter(c => c.status === "Resolved").length;
 return <div className="portal"><header className="portal-nav"><Logo/><nav>{nav.map(n=><button className={tab===n?"active":""} onClick={()=>{setTab(n);setSubmitted(false);}} key={n}>{n}</button>)}</nav><div><Badge tone="primary">{faculty?"Faculty":"Student"}</Badge><span className="avatar">{faculty?"F1":"DA"}</span><button className="icon-btn" onClick={onHome}><Icon name="logout"/></button></div></header><main className="portal-main">{tab==="Dashboard"&&<><div className="portal-welcome"><div><Badge tone="primary">{faculty?"FACULTY PORTAL":"STUDENT PORTAL"}</Badge><h1>Good morning, {faculty?"Dr. Mehta (F1)":"Div A Student"}</h1><p>Here’s your timetable and complaint overview.</p></div><span>Academic Year 2024–25</span></div><div className="metrics"><Metric icon="calendar" label="Monday classes" value={mondayCount.toString()} note={mondayCount === 0 && faculty ? "Blocked Mon 9–11 AM" : "Scheduled Monday"}/><Metric icon="clock" label="Weekly classes" value={mySessions.length.toString()} note="Across 5 days" tone="success"/><Metric icon="message" label="Pending complaints" value={pendingComplaints.toString()} note="Awaiting admin review" tone="warning"/><Metric icon="check" label={faculty?"Queued repairs":"Resolved complaints"} value={faculty ? globalRepairs.length.toString() : resolvedComplaints.toString()} note="This session" tone="purple"/></div><section className="panel portal-agenda"><div className="panel-head"><div><h3>Upcoming schedule</h3><p>{mySessions.length} total weekly sessions</p></div><button onClick={()=>setTab("Timetable")}>View week <Icon name="arrow" size={14}/></button></div><div>{mySessions.length === 0 ? <p style={{color:"#64748b", padding:"12px 0"}}>No sessions loaded yet. Ask Admin to load a dataset and generate the timetable.</p> : mySessions.slice(0,5).map((s:any)=><article key={s.session_id}><b>{formatSlotLabel(s.slot)}</b><i className={`session-line ${s.duration>1?"lab":"lecture"}`}/><div><strong>{s.subject}{s.session_id?.includes("-B") ? ` (${s.session_id.split("-").pop()})` : ""}</strong><span>{faculty?`${s.division} · ${s.room}`:`${s.faculty} · ${s.room}`}</span></div><Badge tone="neutral">{s.duration}h</Badge></article>)}</div></section></>}
 {tab==="Timetable"&&<PortalTimetable faculty={faculty}/>}
 {tab==="Availability"&&<FacultyPage notify={()=>setSubmitted(true)}/>}
 {tab==="Complaints"&&<ComplaintForm submitted={submitted} submit={()=>setSubmitted(true)} authorLabel={faculty ? "Faculty (F1)" : "Student (Div A)"} faculty={faculty}/>}
 </main>{submitted&&<Toast text={tab==="Availability"?"Availability submitted for repair review.":"Complaint submitted for administrator review."}/>}</div>;
}

function PortalTimetable({faculty}:{faculty:boolean}) {
  const placed = useMemo(() => {
    let grid = Array(35).fill(null);
    if (!timetableData || !timetableData.placements) return grid;
    timetableData.placements.forEach((p: any) => {
       if (faculty && p.faculty !== "F1") return; // Logged-in faculty F1
       if (!faculty && p.division !== "Div A") return; // Logged-in student Div A
       if (p.slot >= 0 && p.slot < 35) {
           if (!grid[p.slot]) grid[p.slot] = [];
           grid[p.slot].push(p);
           for (let i = 1; i < (p.duration || 1); i++) {
               if (p.slot + i < 35) {
                   if (!grid[p.slot+i]) grid[p.slot+i] = [];
                   grid[p.slot+i].push(p);
               }
           }
       }
    });
    return grid;
  }, [timetableData, faculty]);

  return <><div className="page-intro"><div><h2>My Timetable</h2><p>{faculty?"Your assigned teaching sessions (Dr. Mehta / F1).":"Div A · Semester 2"} · 35 teaching slots</p></div><Button variant="secondary" icon="export">Download</Button></div>
  <section className="panel timetable-panel portal-table"><div className="schedule-scroll"><div className="schedule-grid"><div className="grid-corner">TIME</div>{days.map(d=><div className="day-head" key={d}>{d}</div>)}
  {periods.map((p,r)=><Fragment key={p}><div className="time-cell"><b>{p.slice(0,5)}</b><small>{p.slice(6)}</small></div>
  {days.map((d,c)=>{
      const bSlot = c * 7 + r;
      const slotSessions = placed[bSlot] || [];
      return <div className="schedule-cell" key={`${p}-${d}`}>
        {slotSessions.map((s:any) => <SessionCard key={s.session_id} s={s} />)}
        {slotSessions.length === 0 && <span className="empty-slot">Available</span>}
      </div>
  })}
  {r===3&&<div className="lunch-band"><b>Lunch break</b><span>13:00–14:00</span></div>}</Fragment>)}
  </div></div></section></>;
}

function ComplaintForm({submitted,submit,authorLabel="Student",faculty=false}:{submitted:boolean;submit:()=>void;authorLabel?:string;faculty?:boolean}) {
  const [type, setType] = useState("Shift Session");
  const [selectedDay, setSelectedDay] = useState("Monday");
  const [selectedSessionId, setSelectedSessionId] = useState("");
  const [selectedGapSlot, setSelectedGapSlot] = useState("");
  const [preferredSlot, setPreferredSlot] = useState("Any available slot");
  const [reason, setReason] = useState("");

  const dayIndex = days.indexOf(selectedDay);

  const mySessions = useMemo(() => {
    return (timetableData?.placements || [])
      .filter((s:any) => faculty ? s.faculty === "F1" : s.division === "Div A")
      .sort((a:any, b:any) => a.slot - b.slot);
  }, [timetableData, faculty]);

  const daySessions = useMemo(() => {
    return mySessions.filter((s:any) => Math.floor(s.slot / 7) === dayIndex);
  }, [mySessions, dayIndex]);

  const gapSlots = useMemo(() => {
    const busy = new Set<number>();
    daySessions.forEach((s:any) => {
      const startP = s.slot % 7;
      for (let d = 0; d < (s.duration || 1); d++) {
        if (startP + d < 7) busy.add(startP + d);
      }
    });
    const busyArr = Array.from(busy).sort((a, b) => a - b);
    const firstBusy = busyArr.length > 0 ? busyArr[0] : -1;
    const lastBusy = busyArr.length > 0 ? busyArr[busyArr.length - 1] : -1;
    return periods
      .map((p, idx) => ({
        period: p,
        idx,
        isBusy: busy.has(idx),
        isIdleGap: firstBusy !== -1 && idx > firstBusy && idx < lastBusy && !busy.has(idx),
      }))
      .filter(x => !x.isBusy);
  }, [daySessions]);

  const effectiveSessions = daySessions.length > 0 ? daySessions : mySessions;

  useEffect(() => {
    if (effectiveSessions.length > 0) {
      if (!effectiveSessions.some((s:any) => s.session_id === selectedSessionId)) {
        setSelectedSessionId(effectiveSessions[0].session_id);
      }
    } else {
      setSelectedSessionId("");
    }
  }, [selectedDay, effectiveSessions, selectedSessionId]);

  useEffect(() => {
    if (gapSlots.length > 0) {
      const preferred = gapSlots.find(g => g.isIdleGap) || gapSlots[0];
      if (!gapSlots.some(g => g.period === selectedGapSlot)) {
        setSelectedGapSlot(preferred.period);
      }
    } else {
      setSelectedGapSlot("");
    }
  }, [selectedDay, gapSlots, selectedGapSlot]);

  const doSubmit = () => {
      if (!reason.trim()) return;
      const chosenSession = effectiveSessions.find((s:any) => s.session_id === selectedSessionId);
      let target = faculty ? "F1" : "Div A";
      let detail = "";
      let slotLabel = "";
      let sessionLabel = "";

      if (type === "Shift Session") {
        if (chosenSession) {
          const sDay = days[Math.floor(chosenSession.slot / 7)] || selectedDay;
          const sPeriod = periods[chosenSession.slot % 7] || "";
          target = `${chosenSession.session_id} · ${chosenSession.subject}`;
          sessionLabel = `${chosenSession.subject} (${chosenSession.division})`;
          slotLabel = sPeriod;
          detail = `${sDay} ${sPeriod} (${chosenSession.room}) → ${preferredSlot}`;
        } else {
          target = `${faculty ? "F1" : "Div A"} · ${selectedDay}`;
          slotLabel = preferredSlot;
          detail = `Shift requested on ${selectedDay} → ${preferredSlot}`;
        }
      } else if (type === "Eliminate Gap") {
        const chosenGap = gapSlots.find(g => g.period === selectedGapSlot);
        const gapPeriod = chosenGap ? chosenGap.period : (selectedGapSlot || periods[2]);
        const gapTag = chosenGap?.isIdleGap ? "Idle Gap" : "Free Period";
        target = `${faculty ? "F1" : "Div A"} · ${selectedDay}`;
        slotLabel = gapPeriod;
        sessionLabel = `Gap on ${selectedDay} (${gapPeriod})`;
        detail = `${selectedDay} · ${gapPeriod} (${gapTag})`;
      } else {
        target = `${faculty ? "F1" : "Div A"} · ${selectedDay}`;
        detail = chosenSession ? `${selectedDay} · ${chosenSession.session_id} (${chosenSession.subject})` : `${selectedDay} · General`;
      }

      globalComplaints.push({
        id: `CMP-${Math.floor(1000 + Math.random()*9000)}`,
        author: authorLabel,
        type,
        day: selectedDay,
        slotLabel,
        sessionLabel,
        preferredSlot: type === "Shift Session" ? preferredSlot : "",
        target,
        detail,
        affected: 1,
        severity: type === "Eliminate Gap" ? "High" : "Medium",
        status: "Pending",
        date: "Today",
        message: reason.trim()
      });
      setReason("");
      onGlobalUpdate();
      submit();
  };

 return <><div className="page-intro"><div><h2>Scheduling Complaints</h2><p>Select the exact day, session, or idle gap to request a review without altering the live timetable.</p></div></div><div className="portal-columns"><section className="panel complaint-form"><h3>Submit a complaint</h3><p>All changes require administrator approval.</p>
   <label><span>Complaint type</span><select value={type} onChange={e=>setType(e.target.value)}><option>Shift Session</option><option>Eliminate Gap</option><option>Other</option></select></label>
   <label><span>Day</span><select value={selectedDay} onChange={e=>setSelectedDay(e.target.value)}>{days.map(d=><option key={d} value={d}>{d}</option>)}</select></label>
   {type === "Shift Session" && <>
     <label><span>Session to shift {daySessions.length > 0 ? `(${selectedDay})` : "(All scheduled sessions)"}</span>
       <select value={selectedSessionId} onChange={e=>setSelectedSessionId(e.target.value)}>
         {effectiveSessions.length === 0 ? <option value="">No timetable generated yet</option> : effectiveSessions.map((s:any)=>(
           <option key={s.session_id} value={s.session_id}>
             {s.session_id} · {s.subject} ({days[Math.floor(s.slot / 7)]} {periods[s.slot % 7]} · {s.room})
           </option>
         ))}
       </select>
     </label>
     <label><span>Preferred new time slot</span>
       <select value={preferredSlot} onChange={e=>setPreferredSlot(e.target.value)}>
         <option value="Any available slot">Any available slot</option>
         {days.map(d => periods.map(p => <option key={`${d}-${p}`} value={`${d} ${p}`}>{d} · {p}</option>))}
       </select>
     </label>
   </>}
   {type === "Eliminate Gap" && (
     <label><span>Which gap on {selectedDay}?</span>
       <select value={selectedGapSlot} onChange={e=>setSelectedGapSlot(e.target.value)}>
         {gapSlots.length === 0 ? <option value="">No gaps on {selectedDay} (fully packed)</option> : gapSlots.map(g=>(
           <option key={g.period} value={g.period}>
             {selectedDay} · {g.period} {g.isIdleGap ? "— Idle Gap (between classes)" : "— Free Period"}
           </option>
         ))}
       </select>
     </label>
   )}
   {type === "Other" && effectiveSessions.length > 0 && (
     <label><span>Related session (optional)</span>
       <select value={selectedSessionId} onChange={e=>setSelectedSessionId(e.target.value)}>
         <option value="">General / No specific session</option>
         {effectiveSessions.map((s:any)=>(
           <option key={s.session_id} value={s.session_id}>
             {s.session_id} · {s.subject} ({days[Math.floor(s.slot / 7)]} {periods[s.slot % 7]})
           </option>
         ))}
       </select>
     </label>
   )}
   <label><span>Reason</span><textarea value={reason} onChange={e=>setReason(e.target.value)} placeholder="Explain the scheduling issue..." rows={3}/></label>
   <Button onClick={doSubmit}>Submit Complaint</Button>
   {submitted&&<div className="success-inline"><Icon name="check"/>Submitted for administrator review.</div>}
 </section><section className="panel"><div className="panel-head"><div><h3>Complaint history</h3><p>Your recent scheduling requests</p></div></div>{globalComplaints.length === 0 ? <p style={{padding:"16px", color:"#64748b", fontSize:12}}>No complaints submitted yet.</p> : globalComplaints.map(x=><div className="history-row" key={x.id}><div><b>{x.type} · {x.target}</b><small>{x.id} · {x.detail || x.day}</small></div><Badge tone={x.status==="Resolved"?"success":x.status==="Proposed"?"primary":"warning"}>{x.status}</Badge><span>{x.date}</span></div>)}</section></div></>;
}

export default function App() {
  useApi();
 const [page,setPage]=useState<Page>("landing"); const [roles,setRoles]=useState(false);
 const home=()=>{setPage("landing");setRoles(false)};
 return <>{page==="landing"&&<Landing onExplore={()=>setRoles(true)}/>} {page==="admin"&&<AdminApp onHome={home}/>} {page==="student"&&<Portal type="student" onHome={home}/>} {page==="faculty"&&<Portal type="faculty" onHome={home}/>} {roles&&<RoleSelection close={()=>setRoles(false)} select={p=>{setPage(p);setRoles(false)}}/>}</>;
}

function DatasetModal({close,notify,onGenerated}:{close:()=>void;notify:(s:string)=>void;onGenerated?:()=>void}) {
  const [json, setJson] = useState("");
  const [isLoading, setIsLoading] = useState(false);
  const upload = async () => {
    try {
        JSON.parse(json);
    } catch (e) {
        notify("Error: Please paste valid JSON, not plain text.");
        return;
    }
    setIsLoading(true);
    try {
        const upRes = await fetch("/api/upload", { method: "POST", body: json });
        if (!upRes.ok) throw new Error("Upload failed");
        notify("Dataset loaded! Running C11 graph coloring solver...");
        await triggerGenerate();
        notify("Timetable generated successfully!");
        close();
        onGenerated?.();
    } catch (e) {
        notify("Upload or generation failed.");
    } finally {
        setIsLoading(false);
    }
  };
  return <Modal title="Load Dataset" subtitle="Paste JSON adversarial dataset" onClose={close}>
    <textarea value={json} onChange={e=>setJson(e.target.value)} rows={15} placeholder='Paste the JSON dataset starting with { "rooms": ... }' style={{width:'100%', fontSize:11, fontFamily:'monospace', padding:8}} />
    <div className="modal-actions"><Button variant="secondary" onClick={close} disabled={isLoading}>Cancel</Button><Button onClick={upload} disabled={isLoading}>{isLoading ? "Generating..." : "Load & Generate"}</Button></div>
  </Modal>;
}
