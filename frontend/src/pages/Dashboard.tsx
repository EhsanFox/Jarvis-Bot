import React, { useState } from "react";
import { motion, AnimatePresence } from "framer-motion";
import DarkModeToggle from "../components/DarkModeToggle";
import PixelEye from "../components/pixelEye";
import { useNavigate } from "react-router-dom";

type Section = "webserver" | "password" | "wifi";

const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const [activeSection, setActiveSection] = useState<Section>("webserver");
  const [webServerUrl, setWebServerUrl] = useState("");
  const [password, setPassword] = useState("");
  const [confirmPassword, setConfirmPassword] = useState("");

  const handleSaveWebServer = () => {
    console.log("Saving WebServer URL:", webServerUrl);
    alert("WebServer URL saved successfully!");
  };

  const handleChangePassword = () => {
    if (!password || !confirmPassword) {
      alert("Please fill in both fields.");
      return;
    }
    if (password !== confirmPassword) {
      alert("Passwords do not match.");
      return;
    }
    alert("Password changed successfully!");
  };

  const handleWifiSetup = () => {
    navigate("/wizard"); // reuse your setup wizard page
  };

  return (
    <div className="min-h-screen flex bg-neutral-100 dark:bg-neutral-950 text-neutral-900 dark:text-neutral-100 transition-colors duration-300">
      {/* Sidebar */}
      <div className="w-64 bg-white dark:bg-neutral-900 border-r border-neutral-200 dark:border-neutral-800 flex flex-col justify-between p-6">
        <div>
          {/* Pixel Eyes */}
          <div className="flex justify-center mb-8">
            <div className="flex gap-4">
              <PixelEye />
              <PixelEye winking />
            </div>
          </div>

          {/* Menu */}
          <nav className="space-y-2">
            <button
              onClick={() => setActiveSection("webserver")}
              className={`w-full text-left px-4 py-2 rounded-lg transition ${
                activeSection === "webserver"
                  ? "bg-blue-500 text-white"
                  : "hover:bg-neutral-200 dark:hover:bg-neutral-800"
              }`}
            >
              WebServer URL
            </button>
            <button
              onClick={() => setActiveSection("password")}
              className={`w-full text-left px-4 py-2 rounded-lg transition ${
                activeSection === "password"
                  ? "bg-blue-500 text-white"
                  : "hover:bg-neutral-200 dark:hover:bg-neutral-800"
              }`}
            >
              Change Password
            </button>
            <button
              onClick={() => setActiveSection("wifi")}
              className={`w-full text-left px-4 py-2 rounded-lg transition ${
                activeSection === "wifi"
                  ? "bg-blue-500 text-white"
                  : "hover:bg-neutral-200 dark:hover:bg-neutral-800"
              }`}
            >
              Wi-Fi Setup
            </button>
          </nav>
        </div>

        {/* Bottom Controls */}
        <div className="flex justify-between items-center mt-8">
          <button
            onClick={() => navigate("/login")}
            className="p-2 rounded-full bg-neutral-200 dark:bg-neutral-800 hover:scale-110 transition"
          >
            ←
          </button>
          <DarkModeToggle />
        </div>
      </div>

      {/* Main Content */}
      <div className="flex-1 p-10">
        <AnimatePresence mode="wait">
          {activeSection === "webserver" && (
            <motion.div
              key="webserver"
              initial={{ opacity: 0, y: 30 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -30 }}
              transition={{ duration: 0.3 }}
            >
              <h2 className="text-2xl font-bold mb-4">
                WebServer Configuration
              </h2>
              <p className="text-neutral-500 mb-4">
                Update your device's web server URL below.
              </p>
              <input
                type="text"
                placeholder="http://192.168.1.10"
                value={webServerUrl}
                onChange={(e) => setWebServerUrl(e.target.value)}
                className="w-full p-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              <button
                onClick={handleSaveWebServer}
                className="mt-6 bg-blue-500 hover:bg-blue-600 text-white px-6 py-2 rounded-lg transition"
              >
                Save
              </button>
            </motion.div>
          )}

          {activeSection === "password" && (
            <motion.div
              key="password"
              initial={{ opacity: 0, y: 30 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -30 }}
              transition={{ duration: 0.3 }}
            >
              <h2 className="text-2xl font-bold mb-4">Change Password</h2>
              <p className="text-neutral-500 mb-4">
                Enter and confirm your new password below.
              </p>
              <input
                type="password"
                placeholder="New Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                className="w-full p-3 mb-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              <input
                type="password"
                placeholder="Confirm Password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
                className="w-full p-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              <button
                onClick={handleChangePassword}
                className="mt-6 bg-blue-500 hover:bg-blue-600 text-white px-6 py-2 rounded-lg transition"
              >
                Change Password
              </button>
            </motion.div>
          )}

          {activeSection === "wifi" && (
            <motion.div
              key="wifi"
              initial={{ opacity: 0, y: 30 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -30 }}
              transition={{ duration: 0.3 }}
            >
              <h2 className="text-2xl font-bold mb-4">Wi-Fi Setup</h2>
              <p className="text-neutral-500 mb-6">
                Open the setup wizard to connect your device to a Wi-Fi network.
              </p>
              <button
                onClick={handleWifiSetup}
                className="bg-blue-500 hover:bg-blue-600 text-white px-6 py-2 rounded-lg transition"
              >
                Open Setup Wizard
              </button>
            </motion.div>
          )}
        </AnimatePresence>
      </div>
    </div>
  );
};

export default Dashboard;
