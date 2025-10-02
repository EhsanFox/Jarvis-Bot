import React from "react";
import { useTheme } from "../providers/ThemeProvider";
import { FiSun, FiMoon } from "react-icons/fi"; // you need react-icons

const DarkModeToggle: React.FC = () => {
  const { dark, toggleDark } = useTheme();

  return (
    <button
      onClick={toggleDark}
      className="flex items-center justify-center w-12 h-12 rounded-full bg-gray-200 dark:bg-gray-800 shadow-md transition-colors duration-500 relative"
    >
      <span
        className={`absolute transition-transform duration-500 ${
          dark ? "rotate-0" : "rotate-180"
        }`}
      >
        {dark ? (
          <FiMoon size={24} className="text-blue-300" />
        ) : (
          <FiSun size={24} className="text-blue-500" />
        )}
      </span>
    </button>
  );
};

export default DarkModeToggle;
