import React from "react";
import {
  BrowserRouter as Router,
  Routes,
  Route,
  Navigate,
} from "react-router-dom";
import Landing from "./pages/Landing";
import Login from "./pages/Login";
// import Setup from "./pages/Setup";
//import DashboardLanding from "./pages/dashboard/DashboardLanding";
//wimport DashboardConfig from "./pages/dashboard/DashboardConfig";

const AppRoutes: React.FC = () => {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<Landing name="Jarvis" />} />

        <Route path="/login" element={<Login />} />
        {/*}
        <Route path="/setup" element={<Setup />} />
        <Route path="/dashboard" element={<DashboardLanding />} />
              <Route path="/dashboard/config" element={<DashboardConfig />} />
        */}
        <Route path="*" element={<Navigate to="/" />} />
      </Routes>
    </Router>
  );
};

export default AppRoutes;
