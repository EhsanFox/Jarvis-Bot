/* eslint-disable @typescript-eslint/no-explicit-any */
import axios, { AxiosError } from "axios";

export interface ApiResponse<T = any> {
  ok: boolean;
  data: T | string;
}

// ✅ Base URL config
const BASE_URL = import.meta.env.VITE_API_URL || `${window.location.origin}`;

const api = axios.create({
  baseURL: BASE_URL,
  withCredentials: true, // keep cookies/session
  headers: { "Content-Type": "application/json" },
});

// ✅ Handle GET requests
export const getApi = async <T = any>(
  endpoint: string
): Promise<ApiResponse<T>> => {
  try {
    const res = await api.get<ApiResponse<T>>(endpoint);
    return res.data;
  } catch (err) {
    return handleAxiosError(err);
  }
};

// ✅ Handle POST requests
export const postApi = async <T = any>(
  endpoint: string,
  body: any
): Promise<ApiResponse<T>> => {
  try {
    const res = await api.post<ApiResponse<T>>(endpoint, body);
    return res.data;
  } catch (err) {
    return handleAxiosError(err);
  }
};

// ✅ Centralized error handler
const handleAxiosError = (err: unknown): ApiResponse => {
  if (axios.isAxiosError(err)) {
    const error = err as AxiosError<ApiResponse>;
    return {
      ok: false,
      data:
        error.response?.data?.data ||
        error.message ||
        "Something went wrong while calling the API.",
    };
  }
  return { ok: false, data: "Unknown error occurred." };
};

export default api;
