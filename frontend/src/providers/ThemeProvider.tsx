import React, { createContext, useContext, useEffect, useState } from "react";
import type { ReactNode } from "react";

interface ThemeContextType {
  dark: boolean;
  toggleDark: () => void;
}

const ThemeContext = createContext<ThemeContextType | undefined>(undefined);

export const ThemeProvider: React.FC<{ children: ReactNode }> = ({
  children,
}) => {
  const [dark, setDark] = useState<boolean>(() => {
    // read from localStorage or fallback to system preference
    if (typeof window !== "undefined") {
      const stored = localStorage.getItem("dark");
      if (stored) return stored === "true";
      return window.matchMedia("(prefers-color-scheme: dark)").matches;
    }
    return false;
  });

  // Sync dark class on <html> whenever state changes
  useEffect(() => {
    const root = document.documentElement;
    if (dark) root.classList.add("dark");
    else root.classList.remove("dark");
    localStorage.setItem("dark", dark.toString());
  }, [dark]);

  const toggleDark = () => setDark((prev) => !prev);

  return (
    <ThemeContext.Provider value={{ dark, toggleDark }}>
      {children}
    </ThemeContext.Provider>
  );
};

// eslint-disable-next-line react-refresh/only-export-components
export const useTheme = (): ThemeContextType => {
  const context = useContext(ThemeContext);
  if (!context) throw new Error("useTheme must be used within ThemeProvider");
  return context;
};
