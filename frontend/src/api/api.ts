/* eslint-disable @typescript-eslint/no-explicit-any */
export interface ApiResponse<T = any> {
  success: boolean;
  message?: string;
  data?: T;
}

const BASE_URL = import.meta.env.VITE_API_URL || "http://localhost:5000";

export const getApi = async <T = any>(
  endpoint: string
): Promise<ApiResponse<T>> => {
  try {
    const res = await fetch(`${BASE_URL}${endpoint}`, {
      method: "GET",
      credentials: "include",
    });
    return res.json();
  } catch (err: any) {
    return { success: false, message: err.message };
  }
};

export const postApi = async <T = any>(
  endpoint: string,
  body: any
): Promise<ApiResponse<T>> => {
  try {
    const res = await fetch(`${BASE_URL}${endpoint}`, {
      method: "POST",
      credentials: "include",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
    return res.json();
  } catch (err: any) {
    return { success: false, message: err.message };
  }
};
