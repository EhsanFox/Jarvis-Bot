/* eslint-disable @typescript-eslint/no-explicit-any */
import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import PixelEye from "../components/pixelEye";
import DarkModeToggle from "../components/DarkModeToggle";
import { motion, AnimatePresence } from "framer-motion";
import { getApi, postApi } from "../api/api";
import { useNotification } from "../providers/NotificationProvider";

interface WifiNetwork {
  ssid: string;
  rssi: number;
  secure: boolean;
  hidden?: boolean;
}

interface ConnectionResult {
  ip: string;
  accessToken: string;
}

type Step = "setPassword" | "scan" | "wifiPassword" | "connecting" | "done";

const Wizard: React.FC = () => {
  const navigate = useNavigate();
  const [step, setStep] = useState<Step>("setPassword");
  const [loginPassword, setLoginPassword] = useState("");
  const [confirmPassword, setConfirmPassword] = useState("");
  const [wifiList, setWifiList] = useState<WifiNetwork[]>([]);
  const [selectedWifi, setSelectedWifi] = useState<WifiNetwork | null>(null);
  const [wifiPassword, setWifiPassword] = useState("");
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [connectionResult, setConnectionResult] =
    useState<ConnectionResult | null>(null);

  const notif = useNotification();

  // Fetch Wi-Fi list
  useEffect(() => {
    if (step !== "scan") return;
    const fetchWifiList = async () => {
      try {
        setLoading(true);
        const res = await getApi<WifiNetwork[]>("/wifi/list");
        if (res.ok) setWifiList((res.data as WifiNetwork[]) || []);
        else throw new Error(res.data as string);
      } catch (err: any) {
        console.error(err);
        setError("Failed to scan Wi-Fi networks.");
        notif.notify(
          (err as string) || "Failed to scan Wi-Fi networks.",
          "error"
        );
      } finally {
        setLoading(false);
      }
    };
    fetchWifiList();
  }, [step]);

  const handleSetPassword = () => {
    if (!loginPassword || !confirmPassword) {
      setError("Please fill in both fields.");
      return;
    }
    if (loginPassword !== confirmPassword) {
      setError("Passwords do not match.");
      return;
    }
    setError(null);
    setStep("scan");
  };

  const handleWifiSelect = (wifi: WifiNetwork) => {
    setSelectedWifi(wifi);
    if (wifi.secure) setStep("wifiPassword");
    else handleConnect(wifi.ssid, "");
  };

  const handleConnect = async (ssid: string, wifiPass: string) => {
    try {
      setLoading(true);
      setStep("connecting");

      const res = await postApi<ConnectionResult>("/wifi/connect", {
        ssid,
        password: wifiPass,
        authPassword: loginPassword,
      });

      if (!res.ok) throw new Error(res.data as string);
      setConnectionResult(res.data as ConnectionResult);
      setStep("done");
    } catch (err: any) {
      console.error(err);
      setError("Failed to connect. Please try again.");
      notif.notify((err as string) || "Failed to connect.", "error");
      setStep("wifiPassword");
    } finally {
      setLoading(false);
    }
  };

  const handleRedirectToNewIP = () => {
    if (!connectionResult?.ip) return;
    const newURL = `http://${connectionResult.ip}/login`;
    window.location.href = newURL;
  };

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-neutral-100 dark:bg-neutral-950 text-neutral-900 dark:text-neutral-100 transition-colors duration-300">
      <div className="absolute top-4 left-4 flex items-center gap-3">
        <button
          onClick={() => navigate("/")}
          className="p-2 rounded-full bg-neutral-200 dark:bg-neutral-800 hover:scale-110 transition"
        >
          ←
        </button>
        <DarkModeToggle />
      </div>

      <div className="flex gap-6 mb-8">
        <PixelEye />
        <PixelEye winking />
      </div>

      <motion.div
        key={step}
        initial={{ opacity: 0, y: 30 }}
        animate={{ opacity: 1, y: 0 }}
        exit={{ opacity: 0, y: -30 }}
        transition={{ duration: 0.4 }}
        className="bg-white dark:bg-neutral-900 rounded-2xl shadow-xl p-8 w-11/12 max-w-md text-center"
      >
        <h2 className="text-2xl font-bold mb-4">Device Setup</h2>

        <AnimatePresence mode="wait">
          {/* SET PASSWORD */}
          {step === "setPassword" && (
            <motion.div key="setPassword" animate={{ opacity: 1 }}>
              <p className="text-neutral-500 mb-4">
                Set a password for your device login.
              </p>
              <input
                type="password"
                placeholder="Password"
                value={loginPassword}
                onChange={(e) => setLoginPassword(e.target.value)}
                className="w-full p-3 mb-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              <input
                type="password"
                placeholder="Confirm Password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
                className="w-full p-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              {error && <p className="text-red-500 text-sm mt-2">{error}</p>}
              <button
                onClick={handleSetPassword}
                className="mt-6 w-full bg-blue-500 hover:bg-blue-600 text-white rounded-lg py-2 transition"
              >
                Continue
              </button>
            </motion.div>
          )}

          {/* SCAN WIFI */}
          {step === "scan" && (
            <motion.div key="scan" animate={{ opacity: 1 }}>
              {loading && <p>Scanning for networks...</p>}
              {error && <p className="text-red-500">{error}</p>}
              <div className="mt-4 space-y-2 max-h-60 overflow-y-auto">
                {wifiList.map((wifi, i) => (
                  <button
                    key={i}
                    onClick={() => handleWifiSelect(wifi)}
                    className="w-full flex justify-between items-center p-3 rounded-lg bg-neutral-200 dark:bg-neutral-800 hover:bg-neutral-300 dark:hover:bg-neutral-700 transition"
                  >
                    <span>{wifi.ssid || "Unknown"}</span>
                    <span className="text-sm opacity-70">
                      {wifi.secure ? "🔒" : "🔓"} {wifi.rssi}%
                    </span>
                  </button>
                ))}
              </div>
              <button
                onClick={() => setStep("scan")}
                className="mt-4 text-blue-500 hover:underline"
              >
                Rescan
              </button>
            </motion.div>
          )}

          {/* WIFI PASSWORD */}
          {step === "wifiPassword" && selectedWifi && (
            <motion.div key="wifiPassword" animate={{ opacity: 1 }}>
              <p className="mb-4 text-neutral-500">
                Enter password for <strong>{selectedWifi.ssid}</strong>
              </p>
              <input
                type="password"
                placeholder="Wi-Fi Password"
                value={wifiPassword}
                onChange={(e) => setWifiPassword(e.target.value)}
                className="w-full p-3 rounded-lg border border-neutral-300 dark:border-neutral-700 bg-transparent outline-none focus:ring-2 focus:ring-blue-500"
              />
              {error && <p className="text-red-500 text-sm mt-2">{error}</p>}

              <div className="mt-6 flex justify-between">
                <button
                  onClick={() => setStep("scan")}
                  className="px-6 py-2 bg-neutral-300 dark:bg-neutral-700 rounded-lg hover:bg-neutral-400 dark:hover:bg-neutral-600 transition"
                >
                  Back
                </button>
                <button
                  onClick={() => handleConnect(selectedWifi.ssid, wifiPassword)}
                  className="px-6 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg transition"
                  disabled={!wifiPassword.trim()}
                >
                  Connect
                </button>
              </div>
            </motion.div>
          )}

          {/* CONNECTING */}
          {step === "connecting" && (
            <motion.div key="connecting" animate={{ opacity: 1 }}>
              <p>Connecting to Wi-Fi...</p>
              <div className="mt-4 animate-spin rounded-full border-4 border-blue-400 border-t-transparent w-12 h-12 mx-auto" />
            </motion.div>
          )}

          {/* DONE */}
          {step === "done" && connectionResult && (
            <motion.div key="done" animate={{ opacity: 1 }}>
              <p className="text-green-500 font-semibold mb-4">
                ✅ Connected successfully!
              </p>
              <p className="text-neutral-500 mb-6">
                Your new device IP: <strong>{connectionResult.ip}</strong>
              </p>
              <button
                onClick={handleRedirectToNewIP}
                className="w-full bg-blue-500 hover:bg-blue-600 text-white rounded-lg py-2 transition"
              >
                Go to Login
              </button>
            </motion.div>
          )}
        </AnimatePresence>
      </motion.div>
    </div>
  );
};

export default Wizard;
