import React, { useEffect, useState } from "react";
import { useNavigate } from "react-router-dom";
import { getApi } from "../api/api";
import PageSkeleton from "../components/skeletons/PageSkeleton";
import PixelEye from "../components/pixelEye";
import DarkModeToggle from "../components/DarkModeToggle";

interface LandingProps {
  name: string;
}

const Landing: React.FC<LandingProps> = ({ name }) => {
  const [loading, setLoading] = useState(true);
  const [showSetup, setShowSetup] = useState(false);
  const navigate = useNavigate();

  useEffect(() => {
    const checkSetupAvailability = async () => {
      try {
        const res = await getApi<{ setupAvailable: boolean }>("/check-setup");
        setShowSetup(res.data?.setupAvailable || false);
      } catch (err) {
        console.error(err);
      } finally {
        setLoading(false);
      }
    };
    checkSetupAvailability();
  }, []);

  if (loading) return <PageSkeleton />;

  return (
    <div className="min-h-screen w-full flex flex-col items-center justify-center px-4 bg-gray-100 dark:bg-gray-900 text-gray-900 dark:text-gray-100 transition-colors">
      {/* Dark Mode Toggle */}
      <div className="absolute top-6 right-6">
        <DarkModeToggle />
      </div>

      {/* Pixel Eyes */}
      <div className="flex gap-6 mb-8 justify-center">
        <PixelEye />
        <PixelEye winking />
      </div>

      {/* Hero Text */}
      <div className="text-center max-w-3xl w-full">
        <h1 className="text-3xl sm:text-4xl md:text-5xl font-extrabold mb-4">
          Welcome to <span className="text-blue-500">{name}</span>
        </h1>
        <p className="text-base sm:text-lg md:text-xl mb-8 max-w-xl mx-auto">
          Your personalized AI companion at the palm of your hand — a friend who
          listens, understands, and speaks back.
        </p>

        {/* Action Buttons */}
        <div className="flex flex-col sm:flex-row gap-4 justify-center w-full">
          {showSetup && (
            <button
              onClick={() => navigate("/setup")}
              className="px-8 py-3 bg-blue-500 text-white font-semibold rounded-lg shadow-lg hover:bg-blue-600 transition w-full sm:w-auto"
            >
              Setup Wizard
            </button>
          )}
          <button
            onClick={() => navigate("/login")}
            className="px-8 py-3 bg-gray-800 dark:bg-gray-700 text-white font-semibold rounded-lg shadow-lg hover:bg-gray-900 dark:hover:bg-gray-600 transition w-full sm:w-auto"
          >
            Login
          </button>
        </div>
      </div>

      {/* Footer */}
      <p className="mt-12 text-gray-500 dark:text-gray-400 text-sm">
        {name} &copy; {new Date().getFullYear()} — Created by{" "}
        <a href="https://ehsan.js.org">Ehsan Golmakani</a>
      </p>
    </div>
  );
};

export default Landing;
