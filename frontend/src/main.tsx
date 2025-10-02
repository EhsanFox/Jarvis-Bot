import React from "react";
import ReactDOM from "react-dom/client";
import AppRoutes from "./Routes";
import { NotificationProvider } from "./providers/NotificationProvider";
import "./index.css";
import { ThemeProvider } from "./providers/ThemeProvider";

ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <ThemeProvider>
      <NotificationProvider>
        <AppRoutes />
      </NotificationProvider>
    </ThemeProvider>
  </React.StrictMode>
);
