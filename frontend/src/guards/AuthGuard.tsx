import { Navigate } from "react-router-dom";
import Cookies from "js-cookie";

interface AuthGuardProps {
  children: React.ReactNode;
}

const AuthGuard = ({ children }: AuthGuardProps) => {
  const token = Cookies.get("accessToken");

  if (!token) {
    // Not logged in
    return <Navigate to="/login" replace />;
  }

  return <>{children}</>;
};

export default AuthGuard;
