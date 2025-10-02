import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { ArrowLeft } from "lucide-react"; // icon
import DarkModeToggle from "../components/DarkModeToggle";
import PixelEye from "../components/pixelEye";

const Login: React.FC = () => {
  const navigate = useNavigate();
  const [password, setPassword] = useState("");
  const [passwordFocused, setPasswordFocused] = useState(false);

  const handleLogin = (e: React.FormEvent) => {
    e.preventDefault();
    console.log("Logging in with:", { password });
    navigate("/dashboard"); // redirect after login
  };

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-white dark:bg-gray-900 px-4">
      {/* Top bar */}
      <div className="absolute top-6 left-6">
        <button
          onClick={() => navigate("/")}
          className="p-2 rounded-full bg-gray-200 dark:bg-gray-700 hover:bg-gray-300 dark:hover:bg-gray-600 transition"
          aria-label="Go back to landing"
        >
          <ArrowLeft className="w-5 h-5 text-gray-800 dark:text-white" />
        </button>
      </div>

      <div className="absolute top-6 right-6">
        <DarkModeToggle />
      </div>

      {/* Pixel Eyes */}
      <div className="flex gap-6 mb-6 justify-center">
        <PixelEye size={14} rolledUp={passwordFocused} />
        <PixelEye size={14} winking rolledUp={passwordFocused} />
      </div>

      {/* Login card */}
      <div className="w-full max-w-md bg-white dark:bg-gray-800 rounded-2xl shadow-lg p-8">
        <h1 className="text-2xl font-bold text-center text-gray-900 dark:text-white">
          Login to <span className="text-blue-500">AI-Bot</span>
        </h1>
        <p className="text-center text-gray-500 dark:text-gray-400 mt-2 mb-6">
          Welcome back! Please enter your credentials.
        </p>

        <form onSubmit={handleLogin} className="space-y-4">
          {/* Password */}
          <div>
            <label
              htmlFor="password"
              className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1"
            >
              Password
            </label>
            <input
              type="password"
              id="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              onFocus={() => setPasswordFocused(true)} // 👈 focus event
              onBlur={() => setPasswordFocused(false)} // 👈 blur event
              required
              className="w-full px-4 py-2 rounded-lg border border-gray-300 dark:border-gray-600 
              bg-gray-50 dark:bg-gray-700 text-gray-900 dark:text-white focus:ring-2 
              focus:ring-blue-500 focus:border-blue-500 outline-none transition"
              placeholder="••••••••"
            />
          </div>

          {/* Submit */}
          <button
            type="submit"
            className="w-full py-3 px-4 bg-blue-500 hover:bg-blue-600 text-white font-semibold rounded-lg shadow-md transition"
          >
            Sign In
          </button>
        </form>
      </div>
    </div>
  );
};

export default Login;
